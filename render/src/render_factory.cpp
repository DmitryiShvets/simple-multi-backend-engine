#include "render_factory.h"
#include "opengl_renderer.h"
#include "vulkan_renderer.h"

#include <stdexcept>

namespace Render {

std::unique_ptr<IRenderer> RenderFactory::create(API api) {
  switch (api) {
  case API::OpenGL:
    return std::make_unique<OpenGLRenderer>();
  case API::Vulkan:
    return std::make_unique<Vulkan::VulkanRenderer>();
  default:
    throw std::runtime_error("Unknown API");
  }
}
} // namespace Render
