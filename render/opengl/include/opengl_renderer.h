#pragma once

#include "opengl_resource_manager.h"
#include "i_renderer.h"
#include <memory>
#include "render_graph_executor.h"
// Forward-declarations
namespace Render {
class Device;
} // namespace Render
namespace Core {
class SceneView;
}

namespace Render::OpenGL {

// This is the concrete, API-dependent implementation of the IRenderer interface
// for OpenGL.
class OpenGLRenderer final : public IRenderer {
public:
  OpenGLRenderer();
  ~OpenGLRenderer();

  void renderFrame(const Core::SceneView &view) override;
  Device &getRenderDeivce() override { return *m_rhi_device; };

private:
  std::unique_ptr<Device> m_rhi_device;

  // Shared, API-agnostic systems
  std::unique_ptr<RenderGraphExecutor> m_executor;
  OpenglResourceManager m_resource_manager;
};

} // namespace Render::OpenGL
