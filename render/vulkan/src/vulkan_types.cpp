#include "vulkan_types.h"

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
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  attributeDescriptions.push_back(
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
  attributeDescriptions.push_back(
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
  attributeDescriptions.push_back(
      {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
  attributeDescriptions.push_back(
      {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

  return attributeDescriptions;
}
} // namespace Render::Vulkan
