#pragma once
#include "i_renderer.h"
#include <memory>
namespace Render {

enum class API {
  OpenGL,
  Vulkan,
};

class RenderFactory {
public:
  static std::unique_ptr<Render::IRenderer> create(API api);
};
} // namespace Render
