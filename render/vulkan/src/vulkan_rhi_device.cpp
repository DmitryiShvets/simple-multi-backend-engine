#include "vulkan_rhi_device.h"
#include "logger.h"
#include "pipeline_config_registry.h"
#include "render_device.h"
#include "render_types.h"
#include "uniform_set.h"
#include "uniforms.h"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_helpers.h"
#include "vulkan_pipeline.h"
#include "vulkan_pipeline_layout.h"
#include "vulkan_resource_manager.h"
#include <cassert>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>

namespace Render::Vulkan {

VulkanRHIDevice::VulkanRHIDevice(VulkanDevice &device,
                                 VulkanResourceManager &resource_manager,
                                 PipelineConfigRegistry &pl_registry)
    : m_device(device), m_resource_manager(resource_manager),
      m_pl_registry(pl_registry) {}

VulkanRHIDevice::~VulkanRHIDevice() {}

// --- Resource Management ---
RID VulkanRHIDevice::createBuffer(const BufferDesc &desc) {
  // Determine usage flags based on buffer usage type
  vk::BufferUsageFlags usage_flags = toVkBufferUsageFlags(desc.usage);
  // TODO: TEST, MAKE DESSIGIN fallback or error;
  // Default to vertex buffer if no usage specified
  if (!usage_flags) {
    usage_flags = vk::BufferUsageFlagBits::eVertexBuffer;
    Logger::error_log("Unsupported buffer type");
    return RID::INVALID;
  }
  const auto stride = desc.vertex_layout.getStride();
  if (usage_flags == vk::BufferUsageFlagBits::eVertexBuffer && stride == 0) {
    Logger::error_log("Cannot create buffer with zero stride");
    return RID::INVALID;
  }
  auto buffer = std::make_unique<VulkanDataBuffer>(
      m_device, desc.size, stride, 1, usage_flags,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);
  if (desc.initial_data) {
    buffer->map();
    buffer->writeToBuffer(desc.initial_data);
    buffer->unmap();
  }
  RID rid = m_resource_manager.add<VulkanDataBuffer>(std::move(buffer));
  return rid;
}
RID VulkanRHIDevice::createTexture(const TextureDesc &desc) { return RID{}; }
RID VulkanRHIDevice::createSampler(const SamplerDesc &desc) { return RID{}; }

RID VulkanRHIDevice::createDescriptorSetLayout(
    const DescriptorSetLayoutDesc &desc) {
  auto builder = DescriptorSetLayout::Builder(m_device);
  for (const auto &binding : desc.bindings) {
    builder.addBinding(binding.binding, toVkDescriptorType(binding.type),
                       toVkShaderStageFlags(binding.stages), binding.count);
  }
  auto layout = builder.build();
  RID rid = m_resource_manager.add(std::move(layout));
  return rid;
}

RID VulkanRHIDevice::createDescriptorSet(RID layout_rid,
                                         const std::vector<RID> &buffer_rids) {
  // Get layout
  auto layout = m_resource_manager.get_ptr<DescriptorSetLayout>(layout_rid);
  if (!layout) {
    throw std::runtime_error("Invalid descriptor set layout RID");
  }

  // Create descriptor pool (could reuse existing if needed)
  // For simplicity, create a new pool for each set
  auto pool_builder = DescriptorPool::Builder(m_device);
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
  for (size_t i = 0; i < buffer_rids.size(); ++i) {
    auto *buffer = m_resource_manager.get_ptr<VulkanDataBuffer>(buffer_rids[i]);
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
  RID pool_rid = m_resource_manager.add(std::move(pool));
  RID set_rid = m_resource_manager.add<VulkanDescriptorSet>(std::move(ds_set));

  return set_rid;
}

RID VulkanRHIDevice::createPipelineLayout(const PipelineLayoutDesc &desc) {
  std::vector<vk::DescriptorSetLayout> vk_ds_layouts;
  vk_ds_layouts.reserve(desc.descriptor_set_layouts.size());
  for (auto rid : desc.descriptor_set_layouts) {
    auto ds_layout = m_resource_manager.get_ptr<DescriptorSetLayout>(rid);
    if (ds_layout) {
      vk_ds_layouts.push_back(ds_layout->getDescriptorSetLayout());
    } else {
      // Handle error: invalid RID
      throw std::runtime_error(
          "Invalid descriptor set layout RID in createPipelineLayout");
    }
  }

  std::vector<vk::PushConstantRange> vk_push_ranges;
  vk_push_ranges.reserve(desc.push_constant_ranges.size());
  for (const auto &range : desc.push_constant_ranges) {
    vk_push_ranges.push_back({
        toVkShaderStageFlags(range.stages),
        range.offset,
        range.size,
    });
  }

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

  return m_resource_manager.add(std::move(layout_wrapper));
}

RID VulkanRHIDevice::createGraphicsPipeline(const GraphicsPipelineDesc &desc) {
  // 1. Check if a PSO with this name already exists
  RID existing_rid = m_resource_manager.findPSO(desc.name);
  if (existing_rid) {
    return existing_rid;
  }

  // --- If not found, create a new one ---

  // 2. Get the pre-created pipeline layout from the resource manager
  auto pipeline_layout_wrapper =
      m_resource_manager.get_ptr<VulkanPipelineLayout>(
          desc.pipeline_layout_rid);
  if (!pipeline_layout_wrapper) {
    throw std::runtime_error("Invalid pipeline layout RID in createPipeline");
  }
  vk::PipelineLayout vk_pipeline_layout = pipeline_layout_wrapper->getHandle();

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
      // .setCullMode(VK_CULL_MODE_NONE)
      .setFrontFace(vk::FrontFace::eCounterClockwise)
      .enableDepthTest(true)
      .enableDepthWrite(true);

  auto pipeline_config = pipeline_config_builder.build();

  // 4. Find shader paths from the abstract descriptor
  std::string vert_path, frag_path;
  for (const auto &shader_module : desc.shader_modules) {
    if (shader_module.stage == ShaderStage::VERTEX) {
      vert_path = shader_module.file_path;
    } else if (shader_module.stage == ShaderStage::FRAGMENT) {
      frag_path = shader_module.file_path;
    }
  }
  if (vert_path.empty() || frag_path.empty()) {
    throw std::runtime_error(
        "Vertex and Fragment shader paths must be provided");
  }

  // 5. Create the VulkanPipeLine object
  auto pipeline = std::make_unique<VulkanPipeLine>(m_device, *pipeline_config,
                                                   vert_path, frag_path);

  // 6. Store it in the resource manager and register its name
  RID new_rid = m_resource_manager.add(std::move(pipeline));
  m_resource_manager.registerPSO(desc.name, new_rid);

  return new_rid;
}

RID VulkanRHIDevice::createPipeline(const PipelineDesc &desc) {
  // TODO: FIX IT
  return createGraphicsPipeline(desc.pl_desc);
}

RID VulkanRHIDevice::createMaterial(const std::string &material_name,
                                    const UniformSet &material_uniforms) {
  // 1. Check if a material with this name already exists
  RID material_rid = m_resource_manager.findMaterial(material_name);
  if (material_rid) {
    // Material exists, just update uniforms
    auto *mat = m_resource_manager.get_ptr<Material>(material_rid);
    if (mat && mat->render_data.uniforms_buf) {
      // Get layout from registry
      auto *config = m_pl_registry.getByName(material_name);
      if (config) {
        // Pack uniforms and update buffer
        std::vector<uint8_t> packed =
            config->uniform_layout.pack(material_uniforms);
        updateBufferRaw(mat->render_data.uniforms_buf, 0, packed.size(),
                        packed.data());
      }
    }
    return material_rid;
  }

  // 2. Get material config from registry
  PipelineConfig *config = m_pl_registry.getByName(material_name);
  if (!config) {
    throw std::runtime_error("Unknown material type: " + material_name);
  }

  // 3. Create descriptor set layouts for each set
  std::vector<RID> ds_layouts;
  ds_layouts.reserve(config->desc.ds_layouts_desc.size());
  for (const auto &ds_layout_desc : config->desc.ds_layouts_desc) {
    RID ds_layout = createDescriptorSetLayout(ds_layout_desc);
    ds_layouts.push_back(ds_layout);
  }
  // 4. Create pipeline layout with all descriptor set layouts
  config->desc.pl_layout_desc.descriptor_set_layouts = ds_layouts;
  RID pl_layout = createPipelineLayout(config->desc.pl_layout_desc);

  // 5. Update pipeline desc with the created layout
  config->desc.pl_desc.pipeline_layout_rid = pl_layout;

  // 6. Create pipeline
  RID pipeline_rid = createGraphicsPipeline(config->desc.pl_desc);

  // 7. Create material uniform buffer using layout
  const auto &layout = config->uniform_layout;
  std::vector<uint8_t> packed_uniforms = layout.pack(material_uniforms);

  RID mat_uniform_buffer = createBuffer(
      BufferDesc{.size = layout.getTotalSize(),
                 .usage = static_cast<uint32_t>(BufferUsage::UNIFORM_BUFFER),
                 .is_host_visible = true,
                 .initial_data = packed_uniforms.data()});

  // 8. Create descriptor set for material uniforms (Set 1)
  // Find the layout for Set 1 (material uniforms)
  RID mat_ds_layout = ds_layouts.size() > 1 ? ds_layouts[1] : ds_layouts[0];
  RID mat_desc_set = createDescriptorSet(mat_ds_layout, {mat_uniform_buffer});

  // 9. Create descriptor set layout for object uniforms (if used by material)
  RID obj_ds_layout = RID::INVALID;
  if (ds_layouts.size() > 2 &&
      !config->object_uniform_layout.getVariables().empty()) {
    obj_ds_layout = ds_layouts[2];
  }

  // 10. Create material template
  material_rid = m_resource_manager.add(std::make_unique<Material>(
      Material{.name = material_name,
               .render_data = {.pipeline = pipeline_rid,
                               .uniforms_buf = mat_uniform_buffer,
                               .uniforms_ds = mat_desc_set,
                               .object_uniform_ds_layout = obj_ds_layout}}));

  return material_rid;
}

const PipelineConfig *
VulkanRHIDevice::getPipelineConfig(const std::string &material_name) const {
  return m_pl_registry.getByName(material_name);
}

Material *VulkanRHIDevice::getMaterial(RID material_rid) {
  return m_resource_manager.get_ptr<Material>(material_rid);
}

void VulkanRHIDevice::updateBufferRaw(RID rid, size_t offset, size_t size,
                                      const void *data) {
  // Get buffer from resource manager
  auto *buffer = m_resource_manager.get_ptr<VulkanDataBuffer>(rid);
  if (!buffer) {
    return; // Invalid RID
  }
  buffer->map();
  // For host-visible buffers, we can directly write
  buffer->writeToBuffer(const_cast<void *>(data), size, offset);
  buffer->unmap();
}

void VulkanRHIDevice::free(RID rid) { m_resource_manager.free(rid); }

} // namespace Render::Vulkan
