#pragma once

#include "core/gpu_types.h"
#include "renderer.h"

#include "opengl_command_list.h"
#include "opengl_resource_manager.h"
#include "pipeline_config_registry.h"
#include <memory>

// Forward-declarations
namespace ssme::opengl {
class OpenGLCommandList;
}

namespace ssme {

class RenderDevice;
class SceneView;
class Platform;

// Per-frame resources for OpenGL (analogous to Vulkan)
struct OpenGLPerFrameResources {
  RID uniform_buffer;     // UBO for FrameUniforms (view-projection, camera)
  RID descriptor_set_rid; // Descriptor set for Set 0 binding
};

// This is the concrete, API-dependent implementation of the IRenderer interface
// for OpenGL.
class OpenGLRenderer final : public IRenderer {
public:
  OpenGLRenderer(Platform *platform);
  ~OpenGLRenderer();

  void init(ImGuiContext *ctx) override;

  void renderFrame(const SceneView &view, ImDrawData *ui_draw_data) override;

  void destroy() override;

  void waitIdle() const override;

  RenderDevice &getRenderDeivce() override { return *m_rhi_device; };

  GpuBackend getGpuBackend() override;

private:
  void createPerFrameResources();
  void updatePerFrameResources(const SceneView &view);

  GpuBackend m_backend_type = GpuBackend::OpenGL;
  Platform *m_platform;
  std::unique_ptr<RenderDevice> m_rhi_device;

  // Shared, API-agnostic systems
  ImGuiContext *m_imgui_context = nullptr;
  ssme::opengl::OpenglResourceManager m_resource_manager;
  PipelineConfigRegistry m_pl_registry;

  // OpenGL command list for drawing
  std::unique_ptr<ssme::opengl::OpenGLCommandList> m_command_list;

  // Per-frame resources (Set 0: camera/projection)
  RID m_per_frame_ds_layout;
  std::vector<OpenGLPerFrameResources> m_per_frame_resources;
};

} // namespace ssme
