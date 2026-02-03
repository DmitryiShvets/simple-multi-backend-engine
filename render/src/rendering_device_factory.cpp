#include "rendering_device_factory.h"
#include "rendering_device.h"
#include "vulkan_device.h"
#include "vulkan_rendering_device.h"
#include <stdexcept>

// Here we could also include "opengl_device.h" when it exists

namespace Render {

// Overload for simple APIs
std::unique_ptr<RenderingDevice>
RenderingDeviceFactory::create(API api) {
  switch (api) {
  case API::OpenGL: {
    // For example: return std::make_unique<OpenGL::OpenGLDevice>();
    throw std::runtime_error("OpenGL device not yet implemented.");
  }
  case API::Vulkan: {
    throw std::runtime_error(
        "Vulkan API requires a surface creator. Please use the correct 'create' overload.");
  }
  default:
    throw std::runtime_error("Unknown or unsupported API.");
  }
}

// Overload for Vulkan
std::unique_ptr<RenderingDevice>
RenderingDeviceFactory::create(IVulkanSurfaceCreator &creator) {
  auto vk_device = std::make_unique<Vulkan::VulkanDevice>();
  vk_device->initialize(creator);
  return std::make_unique<Vulkan::VulkanRenderingDevice>(
      std::move(vk_device));
}

} // namespace Render
