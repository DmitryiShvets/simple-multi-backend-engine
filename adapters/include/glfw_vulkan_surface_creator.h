#pragma once

#include "i_surface_creator.h"
#include <vector>
#include <vulkan/vulkan.hpp>

// Forward-declare GLFWwindow to avoid including glfw3.h in the header.
// The implementation (.cpp) will have the full include.
struct GLFWwindow;

namespace Window {

class GlfwVulkanSurfaceCreator : public IVulkanSurfaceCreator {
public:
  explicit GlfwVulkanSurfaceCreator(void *native_window);

  std::vector<const char *> getRequiredInstanceExtensions() const override;
  VkSurfaceKHR createWindowSurface(vk::Instance instance) const override;

private:
  GLFWwindow *m_window;
};
} // namespace Window
