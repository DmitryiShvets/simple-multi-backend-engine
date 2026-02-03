#pragma once
#include "i_renderer.h"
#include "simple_render_system.h"
#include "vulkan_rendering_device.h"
#include "vulkan_descriptor_set.h"

namespace Render::Vulkan {

class VulkanRenderer : public IRenderer {
public:
  void initialize(int width, int height) override;
  void destroy() override;
  void drawBundle(const std::string &shader, void *bundle) override;
  void frame(float delta_time) override;
  void resize(int width, int height) override;

  void clear() override;
  void setViewPort(int x, int y, int width, int height) override;
  void setClearColor(float r, float g, float b, float a) override;
};
} // namespace Render::Vulkan
