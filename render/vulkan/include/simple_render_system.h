#pragma once
#include "vulkan_buffer.h"
#include "vulkan_pipeline.h"
#include <memory>
namespace Render::Vulkan {

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;

  static std::vector<VkVertexInputBindingDescription> get_binding_description();
  static std::vector<VkVertexInputAttributeDescription>
  get_attribute_description();

  bool operator==(const Vertex &other) const {
    return position == other.position && color == other.color;
  }
};

class SimpleRenderSystem {
public:
  SimpleRenderSystem(VulkanDevice &device, VkRenderPass render_pass,
                     VkDescriptorSetLayout descriptor_layout);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void render(VkCommandBuffer command_buffer);

  VkPipelineLayout getPipelineLayout() const { return m_pipeline_layout; }

private:
  void create_pipline_layout(VkDescriptorSetLayout descriptor_layout);
  void create_pipline(VkRenderPass render_pass);

  VulkanDevice &m_device;

  std::unique_ptr<VulkanPipeLine> m_pipeline;
  std::unique_ptr<VulkanDataBuffer> m_buffer;
  VkPipelineLayout m_pipeline_layout;
};

} // namespace Render::Vulkan
