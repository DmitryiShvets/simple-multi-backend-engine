#include "core/gpu_types.h"
#include "glfw_gpu_context_creator.h"
#include "utils/logger.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
namespace ssme {

void VulkanGpuContextCreator::prepareWindowCreationHints() const {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

bool VulkanGpuContextCreator::createContext(void *window) const {
  // Vulkan context (VkSurface) is created later via a different strategy
  // No action needed here
  uint32_t version;
  vkEnumerateInstanceVersion(&version);
  // 3 macros to extract version info
  uint32_t major = VK_VERSION_MAJOR(version);
  uint32_t minor = VK_VERSION_MINOR(version);
  uint32_t patch = VK_VERSION_PATCH(version);
  Logger::info_log("Initialized Vulkan version " + std::to_string(major) + "." +
                   std::to_string(minor));
  return true;
}

GpuBackend VulkanGpuContextCreator::getGpuBackend() const {
  return GpuBackend::Vulkan;
}
} // namespace ssme
