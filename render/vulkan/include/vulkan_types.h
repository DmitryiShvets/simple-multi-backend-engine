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

} // namespace Render::Vulkan
