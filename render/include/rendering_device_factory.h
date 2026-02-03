#pragma once

#include "i_surface_creator.h"
#include "rendering_device.h"
#include <memory>

namespace Render {

class RenderingDeviceFactory {
public:
  enum class API {
    OpenGL,
    Vulkan,
  };

  // Overload for APIs that don't require special parameters
  static std::unique_ptr<RenderingDevice> create(API api);

  // Overload for Vulkan, which requires a surface creator
  static std::unique_ptr<RenderingDevice> create(IVulkanSurfaceCreator &creator);
};
} // namespace Render
