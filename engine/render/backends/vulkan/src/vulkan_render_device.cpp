#include "vulkan_render_device.h"
#include "core/resource_types.h"
#include "core/rid.h"
#include "vulkan_gpu_storage.h"
#include "utils/debug_assert.h"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_helpers.h"
#include "vulkan_pipeline.h"
#include "vulkan_pipeline_layout.h"
#include "vulkan_shader_module.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>

namespace ssme::vulkan {

VulkanRenderDevice::VulkanRenderDevice(VulkanDevice &device,
                                       VulkanGpuStorageMT &storage)
    : m_device(device), m_storage(storage) {}

VulkanRenderDevice::~VulkanRenderDevice() {}

//------------------------------------------------------------------------
// ---------------------------- Buffer -----------------------------------
// -----------------------------------------------------------------------

RID VulkanRenderDevice::createBuffer(const BufferDesc &desc, RID id) {
  debug_assert(desc.size > 0, "Buffer size cannot be zero");

  vk::BufferUsageFlags usage_flags = toVkBufferUsageFlags(desc.usage);
  debug_assert(usage_flags != vk::BufferUsageFlags{},
               "Invalid or empty buffer usage");

  auto elment_size = desc.element_size;

  debug_assert(elment_size > 0, " Buffer must have a non-zero stride");

  uint64_t elements_count = desc.size / elment_size;

  debug_assert(elements_count > 0, "Buffer size cannot empty");

  auto buffer = std::make_unique<VulkanBuffer>(
      m_device, elment_size, elements_count, usage_flags,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);

  debug_assert(buffer != nullptr,
               "Failed to allocate VulkanBuffer (Out of memory?)");

  if (desc.initial_data) {
    buffer->map();
    buffer->writeToBuffer(desc.initial_data);
    buffer->unmap();
  }

  if (id.isNull()) {
    id = m_storage.add(std::move(buffer));
  } else {
    m_storage.store(id, std::move(buffer));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void VulkanRenderDevice::destroyBuffer(RID rid) {
  if (rid.isNull())
    return;

  // Get buffer from storage
  auto *buffer = m_storage.get<VulkanBuffer>(rid);
  if (buffer) {
    // Remove from storage (this will call VulkanBuffer destructor)
    m_storage.remove<VulkanBuffer>(rid);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Texture ----------------------------------
// -----------------------------------------------------------------------

RID VulkanRenderDevice::createTexture(const TextureDesc &desc, RID id) {
  return RID{};
}

void VulkanRenderDevice::destroyTexture(RID rid) {
  if (rid.isNull())
    return;
  auto *texture = m_storage.get<VulkanTexture>(rid);
  if (texture) {
    m_storage.remove<VulkanTexture>(rid);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Sampler ----------------------------------
// -----------------------------------------------------------------------

RID VulkanRenderDevice::createSampler(const SamplerDesc &desc, RID id) {
  return RID{};
}

//------------------------------------------------------------------------
// ---------------------------- Descriptor layout ------------------------
// -----------------------------------------------------------------------

RID VulkanRenderDevice::createDescriptorLayout(const DescriptorLayout &desc,
                                               RID id) {
  auto builder = VulkanDescriptorSetLayout::Builder(m_device);
  for (const auto &binding : desc.bindings) {
    builder.addBinding(binding.binding, toVkDescriptorType(binding.type),
                       toVkShaderStageFlags(binding.stages), binding.count);
  }
  auto layout = builder.build();

  if (id.isNull()) {
    id = m_storage.add(std::move(layout));
  } else {
    m_storage.store(id, std::move(layout));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void VulkanRenderDevice::destroyDescriptorLayout(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *ds_layout = m_storage.get<VulkanDescriptorSetLayout>(id);
  if (ds_layout) {
    // Remove from storage (this will call VulkanDescriptorSetLayout destructor)
    m_storage.remove<VulkanDescriptorSetLayout>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Descriptor -------------------------------
// -----------------------------------------------------------------------

RID VulkanRenderDevice::createDescriptor(const DescriptorDesc &desc, RID id) {
  // Get layout
  auto layout = m_storage.get<VulkanDescriptorSetLayout>(desc.layout_id);
  if (!layout) {
    throw std::runtime_error("Invalid descriptor set layout RID");
  }

  // Create descriptor pool (could reuse existing if needed)
  // For simplicity, create a new pool for each set
  auto pool_builder = VulkanDescriptorPool::Builder(m_device);
  pool_builder.setMaxSets(1).setPoolFlags(
      vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

  // Add pool sizes based on layout
  for (const auto &[binding_idx, binding_info] : layout->getBindings()) {
    pool_builder.addPoolSize(binding_info.descriptorType,
                             binding_info.descriptorCount);
  }

  auto pool = pool_builder.build();

  // Create writer and fill bindings
  DescriptorWriter writer(*layout, *pool);

  // Add binding for each buffer
  for (size_t i = 0; i < desc.uniform_buffers.size(); ++i) {
    auto *buffer = m_storage.get<VulkanBuffer>(desc.uniform_buffers[i]);
    if (!buffer) {
      throw std::runtime_error("Invalid buffer RID in createDescriptorSet");
    }

    // Get binding from layout
    auto binding_info = layout->getBindings().at(static_cast<uint32_t>(i));
    uint32_t binding = binding_info.binding;

    // Verify buffer has correct usage flag for the descriptor type
    if (binding_info.descriptorType == vk::DescriptorType::eUniformBuffer) {
      assert(
          (buffer->getUsageFlags() & vk::BufferUsageFlagBits::eUniformBuffer) &&
          "Buffer must be created with VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT "
          "for uniform buffer descriptor");
    }

    vk::DescriptorBufferInfo buffer_info = buffer->getDescriptorInfo();
    writer.writeBuffer(binding, &buffer_info);
  }

  // Allocate and write descriptor set
  auto ds_set = writer.build();
  // Store pool and set in resource manager
  // Pool must be stored to keep descriptor set alive
  RID pool_rid = m_storage.add(std::move(pool));

  if (id.isNull()) {
    id = m_storage.add(std::move(ds_set));
  } else {
    m_storage.store(id, std::move(ds_set));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void VulkanRenderDevice::destroyDescriptor(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *ds = m_storage.get<VulkanDescriptorSet>(id);
  if (ds) {
    // Remove from storage (this will call VulkanDescriptorSet destructor)
    m_storage.remove<VulkanDescriptorSet>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Pipeline layout --------------------------
// -----------------------------------------------------------------------

RID VulkanRenderDevice::createPipelineLayout(const PipelineLayoutDesc &desc,
                                             RID id) {
  std::size_t hash = desc.hash();
  // 1. Check if a PSO with this name already exists
  RID existing_rid = m_storage.findPSOLayout(hash);
  if (existing_rid) {
    debug_assert(
        false, "External error in Resource manager. Atept to create Pipeline "
               "layout that already "
               "existed. Resource manager is not found by PipelimeParams HASH");
    // return existing_rid;
  }
  // collect uniforms
  std::vector<vk::DescriptorSetLayout> vk_ds_layouts;
  vk_ds_layouts.reserve(desc.descriptor_layouts.size());
  for (const RID &id : desc.descriptor_layouts) {
    const auto &ds_layout = m_storage.get<VulkanDescriptorSetLayout>(id);
    if (ds_layout) {
      vk_ds_layouts.push_back(ds_layout->getDescriptorSetLayout());
    } else {
      // Handle error: invalid RID
      throw std::runtime_error(
          "Invalid descriptor set layout RID in createPipelineLayout");
    }
  }
  // collect push constants
  std::vector<vk::PushConstantRange> vk_push_ranges;
  vk_push_ranges.reserve(desc.push_constant_ranges.size());
  for (const auto &range : desc.push_constant_ranges) {
    vk_push_ranges.push_back({
        toVkShaderStageFlags(range.stages),
        range.offset,
        range.size,
    });
  }
  // create layout
  vk::PipelineLayoutCreateInfo pl_create_info{
      .setLayoutCount = static_cast<uint32_t>(vk_ds_layouts.size()),
      .pSetLayouts = vk_ds_layouts.data(),
      .pushConstantRangeCount = static_cast<uint32_t>(vk_push_ranges.size()),
      .pPushConstantRanges = vk_push_ranges.data(),
  };

  vk::raii::PipelineLayout vk_pipeline_layout =
      m_device.getHandle().createPipelineLayout(pl_create_info);

  auto layout_wrapper = std::move(std::make_unique<VulkanPipelineLayout>(
      m_device, std::move(vk_pipeline_layout)));

  if (id.isNull()) {
    id = m_storage.add(std::move(layout_wrapper));
  } else {
    m_storage.store(id, std::move(layout_wrapper));
  }

  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  m_storage.registerPSOLayout(hash, id);

  return id;
}

void VulkanRenderDevice::destroyPipelineLayout(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *layout = m_storage.get<VulkanPipelineLayout>(id);
  if (layout) {
    // Remove from storage (this will call VulkanPipelineLayout destructor)
    m_storage.remove<VulkanPipelineLayout>(id);
  }
}
RID VulkanRenderDevice::containsPipelineLayout(std::size_t hash) {
  RID existing_rid = m_storage.findPSOLayout(hash);
  if (existing_rid) {
    return existing_rid;
  }
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Pipeline ---------------------------------
// -----------------------------------------------------------------------

RID VulkanRenderDevice::createGraphicsPipeline(const GraphicsPipelineDesc &desc,
                                               RID id) {
  std::size_t hash = desc.hash();
  // 1. Check if a PSO with this name already exists
  RID existing_rid = m_storage.findPSO(hash);
  if (existing_rid) {
    debug_assert(
        false,
        "External error in Resource manager. Atept to create PSO that already "
        "existed. Resource manager is not found by PipelimeParams HASH");
    // return existing_rid;
  }

  // --- If not found, create a new one ---

  // 2. Get the pre-created pipeline layout from the resource manager
  auto vk_pl_layout = m_storage.get<VulkanPipelineLayout>(desc.pl_layout_id);
  if (!vk_pl_layout) {
    throw std::runtime_error("Invalid pipeline layout RID in createPipeline");
  }
  vk::PipelineLayout vk_pipeline_layout = vk_pl_layout->getHandle();

  // Debug: Check if handle is valid
  if (!vk_pipeline_layout) {
    throw std::runtime_error(
        "createGraphicsPipeline: vk_pipeline_layout is VK_NULL_HANDLE");
  }

  // 3. Translate abstract desc to Vulkan-specific PipelineConfigInfo
  // this data describes how to pass this data format to the vertex shader
  std::vector<vk::VertexInputBindingDescription> binding_descriptions;
  const auto &bindings = desc.vertex_layout.getBindings();
  for (const auto &binding : bindings) {
    binding_descriptions.push_back({binding.binding, binding.stride});
  }

  std::vector<vk::VertexInputAttributeDescription> attribute_descriptions;
  const auto &attributes = desc.vertex_layout.getAttributes();
  for (const auto &attr : attributes) {
    attribute_descriptions.push_back(
        {attr.location, attr.binding, toVkFormat(attr.format), attr.offset});
  }

  // TODO: Get swapchain format properly
  std::vector<vk::Format> color_formats = {vk::Format::eB8G8R8A8Srgb};

  PipelineConfigInfo::Builder pipeline_config_builder;
  pipeline_config_builder.setPipelineLayout(vk_pipeline_layout)
      .setVertexInputInfo(binding_descriptions, attribute_descriptions)
      .setColorAttachmentFormats(color_formats)
      .setCullMode(vk::CullModeFlagBits::eBack)
      // .setCullMode(vk::CullModeFlagBits::eNone)
      .setFrontFace(vk::FrontFace::eCounterClockwise)
      // .setFrontFace(vk::FrontFace::eClockwise)
      .enableDepthTest(true)
      .enableDepthWrite(true);

  auto pipeline_config = pipeline_config_builder.build();

  // 4. Find shaders paths from the abstract descriptor
  auto vert_shader = m_storage.get<VulkanShaderModule>(desc.vert_shader_module);
  auto frag_shader = m_storage.get<VulkanShaderModule>(desc.frag_shader_module);
  if (!vert_shader || !frag_shader) {
    throw std::runtime_error(
        "Vertex and Fragment shader paths must be provided");
  }

  // 5. Create the VulkanPipeLine object
  auto pipeline = std::make_unique<VulkanPipeLine>(m_device, *pipeline_config,
                                                   *vert_shader, *frag_shader);

  // 6. Store it in the resource manager and register its name
  if (id.isNull()) {
    id = m_storage.add(std::move(pipeline));
  } else {
    m_storage.store(id, std::move(pipeline));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  m_storage.registerPSO(hash, id);

  return id;
}

void VulkanRenderDevice::destroyGraphicsPipeline(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *pipeline = m_storage.get<VulkanPipeLine>(id);
  if (pipeline) {
    // Remove from storage (this will call VulkanPipeLine destructor)
    m_storage.remove<VulkanPipeLine>(id);
  }
}

RID VulkanRenderDevice::containsGraphicsPipeline(std::size_t hash) {
  RID existing_rid = m_storage.findPSO(hash);
  if (existing_rid) {
    return existing_rid;
  }
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Shader -----------------------------------
// -----------------------------------------------------------------------

RID VulkanRenderDevice::createShaderModule(const ShaderModuleDesc &desc,
                                           RID id) {
 auto path = "res/shaders/" + desc.file_path + ".spv";
  auto shader_module =
      std::make_unique<VulkanShaderModule>(m_device, path);
  if (id.isNull()) {
    id = m_storage.add(std::move(shader_module));
  } else {
    m_storage.store(id, std::move(shader_module));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void VulkanRenderDevice::destroyShaderModule(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *shader_module = m_storage.get<VulkanShaderModule>(id);
  if (shader_module) {
    // Remove from storage (this will call VulkanShaderModule destructor)
    m_storage.remove<VulkanShaderModule>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Common -----------------------------------
// -----------------------------------------------------------------------

void VulkanRenderDevice::updateBufferRaw(RID rid, size_t offset, size_t size,
                                         const void *data) {
  // Get buffer from resource manager
  auto *buffer = m_storage.get<VulkanBuffer>(rid);
  if (!buffer) {
    return; // Invalid RID
  }
  buffer->map();
  // For host-visible buffers, we can directly write
  buffer->writeToBuffer(const_cast<void *>(data), size, offset);
  buffer->unmap();
}

} // namespace ssme::vulkan
