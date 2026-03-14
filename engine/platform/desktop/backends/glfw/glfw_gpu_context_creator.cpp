#include "desktop/backends/glfw/glfw_gpu_context_creator.h"
#include "utils/logger.h"
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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

void VulkanGpuContextCreator::prepareWindowCreationHints() const {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

bool VulkanGpuContextCreator::createContext(void *window) const {
  // Vulkan context (VkSurface) is created later via a different strategy
  // No action needed here
  return true;
}

} // namespace ssme
