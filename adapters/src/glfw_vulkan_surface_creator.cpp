#include "glfw_vulkan_surface_creator.h"
#include "logger.h"

// This file requires the full GLFW implementation
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Window {

GlfwVulkanSurfaceCreator::GlfwVulkanSurfaceCreator(void *native_window) {
  m_window = static_cast<GLFWwindow *>(native_window);
}

std::vector<const char *>
GlfwVulkanSurfaceCreator::getRequiredInstanceExtensions() const {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  return extensions;
}

VkSurfaceKHR
GlfwVulkanSurfaceCreator::createWindowSurface(vk::Instance vkInstance) const {
  VkSurfaceKHR surface;
  uint32_t version;
  vkEnumerateInstanceVersion(&version);
  // 3 macros to extract version info
  uint32_t major = VK_VERSION_MAJOR(version);
  uint32_t minor = VK_VERSION_MINOR(version);
  uint32_t patch = VK_VERSION_PATCH(version);
  Logger::info_log("Initialized Vulkan version " + std::to_string(major) + "." +
                   std::to_string(minor));
  glfwCreateWindowSurface(vkInstance, m_window, nullptr, &surface);
  return surface;
}

} // namespace Window
