#include "simple_render_system.h"

#include <cassert>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
struct PushConstantData {
  glm::mat4 modelMatrix{1.0f};
  glm::mat4 normalMatrix{1.0f};
};

namespace Render::Vulkan {

std::vector<VkVertexInputBindingDescription> Vertex::get_binding_description() {
  std::vector<VkVertexInputBindingDescription> binding_decription(1);
  binding_decription[0].binding = 0;
  binding_decription[0].stride = sizeof(Vertex);
  binding_decription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return binding_decription;
}

std::vector<VkVertexInputAttributeDescription>
Vertex::get_attribute_description() {
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions;

  attribute_descriptions.push_back(
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
  attribute_descriptions.push_back(
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});

  return attribute_descriptions;
}

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
  assert(m_pipeline_layout != nullptr &&
         "Cannot create pipeline before pipeline layout");

  auto binding_decription = Vertex::get_binding_description();
  auto attrib_decription = Vertex::get_attribute_description();

  auto pipeline_config =
      PipelineConfigInfo::Builder()
          .setRenderPass(render_pass)
          .setPipelineLayout(m_pipeline_layout)
          .setVertexInputInfo(binding_decription, attrib_decription)
          .build();
  m_pipeline = std::make_unique<VulkanPipeLine>(
      m_device, *pipeline_config, "res/shaders/v_test.vert.spv",
      "res/shaders/f_test.frag.spv");
}

void SimpleRenderSystem::render(VkCommandBuffer command_buffer) {
  m_pipeline->bind_buffer(command_buffer);

  // The pipeline has dynamic viewport and scissor enabled, so we must set them.
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = 800; // These should come from the swapchain extent
  viewport.height = 600;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = {800,
                    600}; // These should also come from the swapchain extent
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);
  vkCmdDraw(command_buffer, 3, 1, 0, 0);
}

} // namespace Render::Vulkan
