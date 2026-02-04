#pragma once
#include "i_renderer.h"
#include <memory>

// Forward-declare the abstract surface creator interface
class IVulkanSurfaceCreator;

namespace Render {

enum class API {
  OpenGL,
  Vulkan,
};

class RenderFactory {
public:
  // Overload for simple backends like OpenGL
  static std::unique_ptr<IRenderer> create(API api);

  // Overload for Vulkan, which requires a surface creator strategy
  static std::unique_ptr<IRenderer> create(IVulkanSurfaceCreator& vk_surface_creator);
};
} // namespace Render
