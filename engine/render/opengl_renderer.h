#pragma once

#include "core/gpu_types.h"
#include "core/resource_types.h"
#include "opengl_gpu_storage.h"
#include "renderer.h"

#include "opengl_command_list.h"
#include <memory>

// Forward-declarations
namespace ssme::opengl {
class OpenGLCommandList;
}

namespace ssme {

class RenderDevice;
class Platform;
class ResourceManager;
class SceneView;

// This is the concrete, API-dependent implementation of the IRenderer interface
// for OpenGL.
class OpenGLRenderer final : public IRenderer {
public:
  OpenGLRenderer(Platform *platform, ResourceManager* rm );
  ~OpenGLRenderer();

  void init(ImGuiContext *ctx) override;

  void renderFrame(SceneView &view, ImDrawData *ui_draw_data) override;

  void destroy() override;

  void waitIdle() const override;

  RenderDevice &getRenderDeivce() override { return *m_rhi_device; };

  GpuBackend getGpuBackend() override;

  void setFrameResources(std::shared_ptr<FrameData> data) override;

private:

  void updatePerFrameResources(const SceneView &view);

  GpuBackend m_backend_type = GpuBackend::OpenGL;
  Platform *m_platform;
  ResourceManager* m_rm;

  std::unique_ptr<RenderDevice> m_rhi_device;

  // Shared, API-agnostic systems
  ImGuiContext *m_imgui_context = nullptr;
  opengl::OpenGLGpuStorageMT m_storage;

  // OpenGL command list for drawing
  std::unique_ptr<ssme::opengl::OpenGLCommandList> m_command_list;

  std::shared_ptr<FrameData> m_frame_data = nullptr;
};

} // namespace ssme
