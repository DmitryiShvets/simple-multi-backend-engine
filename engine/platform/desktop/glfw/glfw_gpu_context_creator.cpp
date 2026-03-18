#include "glfw_gpu_context_creator.h"
#include "core/gpu_types.h"
#include "utils/logger.h"
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
namespace ssme {

void OpenGLGpuContextCreator::prepareWindowCreationHints() const {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

bool OpenGLGpuContextCreator::createContext(void *window) const {
  auto glfw_window = static_cast<GLFWwindow *>(window);
  glfwMakeContextCurrent(glfw_window);

  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) {
    Logger::error_log("Failed to initialize OpenGL context!");
    return false;
  }

  Logger::info_log("Initialized OpenGL version " +
                   std::to_string(GLAD_VERSION_MAJOR(version)) + "." +
                   std::to_string(GLAD_VERSION_MINOR(version)));

  glfwSwapInterval(1);
  return true;
}

GpuBackend OpenGLGpuContextCreator::getGpuBackend() const {
    return GpuBackend::OpenGL;
}

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
  Logger::info_log("Initialized Vulkan version " + std::to_string(major) +
  "." +
                   std::to_string(minor));
  return true;
}

 GpuBackend VulkanGpuContextCreator::getGpuBackend() const {
     return GpuBackend::Vulkan;
 }
} // namespace ssme
