#include "render_factory.h"

// This is now the one "dirty" file that includes all concrete implementation headers
#include "opengl_renderer.h"
#include "vulkan_renderer.h"
#include "vulkan_device.h"
#include "i_surface_creator.h"

#include <stdexcept>
#include <memory>
#include <utility>

namespace Render {

// Overload for simple backends
std::unique_ptr<IRenderer> RenderFactory::create(API api) {
  switch (api) {
  case API::OpenGL:
    return std::make_unique<OpenGL::OpenGLRenderer>();
  case API::Vulkan:
     throw std::runtime_error(
        "Vulkan API requires a surface creator. Please use the correct 'create' overload.");
  default:
    throw std::runtime_error("Unknown API");
  }
}

// Overload for Vulkan
std::unique_ptr<IRenderer> RenderFactory::create(IVulkanSurfaceCreator& vk_surface_creator) {
    // This logic is now correctly placed here.
    // It creates the low-level device...
    auto vk_device = std::make_unique<Vulkan::VulkanDevice>();
    vk_device->initialize(vk_surface_creator);

    // ...and passes it to our Level 3 orchestrator, the VulkanRenderer.
    return std::make_unique<Vulkan::VulkanRenderer>(std::move(vk_device));
}

} // namespace Render
