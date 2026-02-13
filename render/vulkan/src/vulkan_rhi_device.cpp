#include "vulkan_rhi_device.h"
#include "render_types.h"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_pipeline.h"
#include "vulkan_pipeline_layout.h"
#include <cassert>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "render_device.h"
#include "vulkan_resource_manager.h"

namespace Render::Vulkan {

// Helper functions
VkFormat Format_to_VkFormat(Render::Format format) {
  switch (format) {
  case Render::Format::R32G32B32_SFLOAT:
    return VK_FORMAT_R32G32B32_SFLOAT;
  case Render::Format::R32G32_SFLOAT:
    return VK_FORMAT_R32G32_SFLOAT;
  default:
    return VK_FORMAT_UNDEFINED;
  }
}

VkDescriptorType
DescriptorType_to_VkDescriptorType(Render::DescriptorType type) {
  switch (type) {
  case Render::DescriptorType::UNIFORM_BUFFER:
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  case Render::DescriptorType::COMBINED_IMAGE_SAMPLER:
    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // Add other types as needed
  default:
    assert(false && "Unsupported descriptor type");
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
  }
}

VkShaderStageFlags
ShaderStageFlags_to_VkShaderStageFlags(Render::ShaderStageFlags flags) {
  VkShaderStageFlags vk_flags = 0;
  if (flags & static_cast<uint32_t>(Render::ShaderStage::VERTEX)) {
    vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
  }
  if (flags & static_cast<uint32_t>(Render::ShaderStage::FRAGMENT)) {
    vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
  }
  if (flags & static_cast<uint32_t>(Render::ShaderStage::COMPUTE)) {
    vk_flags |= VK_SHADER_STAGE_COMPUTE_BIT;
  }
  return vk_flags;
}

VulkanRHIDevice::VulkanRHIDevice(VulkanDevice &device,
                                 VulkanResourceManager &resource_manager)
    : m_device(device), m_resource_manager(resource_manager) {}

VulkanRHIDevice::~VulkanRHIDevice() {
  // Destruction is handled by unique_ptr in resource manager.
}

// --- Resource Management ---
RID VulkanRHIDevice::createBuffer(const BufferDesc &desc) {
  auto buffer = std::make_unique<VulkanDataBuffer>(
      m_device, desc.size, 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (desc.initial_data) {
    buffer->map();
    buffer->writeToBuffer(desc.initial_data);
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
    builder.addBinding(binding.binding,
                       DescriptorType_to_VkDescriptorType(binding.type),
                       ShaderStageFlags_to_VkShaderStageFlags(binding.stages),
                       binding.count);
  }
  auto layout = builder.build();
  RID rid = m_resource_manager.add(std::move(layout));
  return rid;
}

RID VulkanRHIDevice::createPipelineLayout(const PipelineLayoutDesc &desc) {
  std::vector<VkDescriptorSetLayout> vk_ds_layouts;
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

  std::vector<VkPushConstantRange> vk_push_ranges;
  vk_push_ranges.reserve(desc.push_constant_ranges.size());
  for (const auto &range : desc.push_constant_ranges) {
    vk_push_ranges.push_back({
        ShaderStageFlags_to_VkShaderStageFlags(range.stages),
        range.offset,
        range.size,
    });
  }

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount =
      static_cast<uint32_t>(vk_ds_layouts.size());
  pipelineLayoutInfo.pSetLayouts = vk_ds_layouts.data();
  pipelineLayoutInfo.pushConstantRangeCount =
      static_cast<uint32_t>(vk_push_ranges.size());
  pipelineLayoutInfo.pPushConstantRanges = vk_push_ranges.data();

  VkPipelineLayout vk_pipeline_layout;
  if (vkCreatePipelineLayout(m_device.getDeviceHandle(), &pipelineLayoutInfo,
                             nullptr, &vk_pipeline_layout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  auto layout_wrapper =
      std::make_unique<VulkanPipelineLayout>(m_device, vk_pipeline_layout);

  return m_resource_manager.add(std::move(layout_wrapper));
}

RID VulkanRHIDevice::createGraphicsPipeline(
    const GraphicsPipelineDesc &desc) {
  // 1. Check if a PSO with this name already exists
  RID existing_rid = m_resource_manager.findPSO(desc.name);
  if (existing_rid) {
    return existing_rid;
  }

  // --- If not found, create a new one ---

  // 2. Get the pre-created pipeline layout from the resource manager
  auto pipeline_layout_wrapper =
      m_resource_manager.get_ptr<VulkanPipelineLayout>(desc.pipeline_layout_rid);
  if (!pipeline_layout_wrapper) {
    throw std::runtime_error(
        "Invalid pipeline layout RID in createGraphicsPipeline");
  }
  VkPipelineLayout vk_pipeline_layout = pipeline_layout_wrapper->get();

  // 3. Translate abstract desc to Vulkan-specific PipelineConfigInfo
  std::vector<VkVertexInputBindingDescription> binding_descriptions;
  for (const auto &binding : desc.vertex_input_state.bindings) {
    binding_descriptions.push_back({binding.binding, binding.stride});
  }

  std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
  for (const auto &attr : desc.vertex_input_state.attributes) {
    attribute_descriptions.push_back({attr.location, attr.binding,
                                       Format_to_VkFormat(attr.format),
                                       attr.offset});
  }

  // TODO: Get swapchain format properly
  std::vector<VkFormat> color_formats = {VK_FORMAT_B8G8R8A8_SRGB};

  PipelineConfigInfo::Builder pipeline_config_builder;
  pipeline_config_builder.setPipelineLayout(vk_pipeline_layout)
          .setVertexInputInfo(binding_descriptions, attribute_descriptions)
          .setColorAttachmentFormats(color_formats)
          .setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
          .setCullMode(VK_CULL_MODE_BACK_BIT)
          .setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
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
  auto pipeline = std::make_unique<VulkanPipeLine>(
      m_device, *pipeline_config, vert_path, frag_path);

  // 6. Store it in the resource manager and register its name
  RID new_rid = m_resource_manager.add(std::move(pipeline));
  m_resource_manager.registerPSO(desc.name, new_rid);

  return new_rid;
}

void VulkanRHIDevice::free(RID rid) { m_resource_manager.free(rid); }

} // namespace Render::Vulkan
