#include "simple_render_system.h"

#include <cassert>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
struct PushConstantData {
  glm::mat4 modelMatrix{1.0f};
  int index;
};

namespace Render::Vulkan {

SimpleRenderSystem::SimpleRenderSystem(VulkanDevice &device,
                                       VkRenderPass render_pass,
                                       VkDescriptorSetLayout descriptor_layout)
    : m_device(device) {
  create_pipline_layout(descriptor_layout);
  create_pipline(render_pass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(m_device.getDeviceHandle(), m_pipeline_layout,
                          nullptr);
}

void SimpleRenderSystem::create_pipline_layout(
    VkDescriptorSetLayout descriptor_layout) {
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(PushConstantData);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{descriptor_layout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount =
      static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;                 // Optional
  pipelineLayoutInfo.pPushConstantRanges = &push_constant_range; // Optional

  if (vkCreatePipelineLayout(m_device.getDeviceHandle(), &pipelineLayoutInfo,
                             nullptr, &m_pipeline_layout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void SimpleRenderSystem::create_pipline(VkRenderPass render_pass) {
  // assert(m_swapchain != nullptr && "Cannot create pipeline before swap
  // chain");
  assert(m_pipeline_layout != nullptr &&
         "Cannot create pipeline before pipeline layout");

  m_pipeline =
      std::make_unique<VulkanPipeLine>(m_device, render_pass, m_pipeline_layout,
                                       "test2/res/shaders/texture.vert.spv",
                                       "test2/res/shaders/texture.frag.spv");
}

} // namespace Render::Vulkan
