#pragma once

#include "i_renderer.h"
#include <memory>

// Forward-declarations
namespace Render {
class Device;
class SceneRenderer;
class RenderGraphExecutor;
} // namespace Render
class OpenglResourceManager;
class SceneView;

namespace Render::OpenGL {

// This is the concrete, API-dependent implementation of the IRenderer interface
// for OpenGL.
class OpenGLRenderer final : public IRenderer {
public:
  OpenGLRenderer();
  ~OpenGLRenderer();

  void renderFrame(const SceneView &view) override;

private:
  std::unique_ptr<Device> m_rhi_device;

  // Shared, API-agnostic systems
  std::unique_ptr<SceneRenderer> m_scene_renderer;
  std::unique_ptr<RenderGraphExecutor> m_executor;
  std::unique_ptr<OpenglResourceManager> m_resource_manager;
};

} // namespace Render::OpenGL
