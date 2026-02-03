#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace Render::Vulkan {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct QueueFamilyIndices {
  uint32_t graphics_family;
  uint32_t present_family;
  bool graphics_family_has_value = false;
  bool present_family_has_value = false;
  bool isComplete() {
    return graphics_family_has_value && present_family_has_value;
  }
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec3 normal;
  glm::vec2 uv;

  static std::vector<VkVertexInputBindingDescription> get_binding_description();
  static std::vector<VkVertexInputAttributeDescription>
  get_attribute_description();

  bool operator==(const Vertex &other) const {
    return position == other.position && color == other.color &&
           normal == other.normal && uv == other.uv;
  }
};

} // namespace Render::Vulkan
