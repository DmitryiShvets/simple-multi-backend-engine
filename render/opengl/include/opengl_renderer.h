#pragma once

#include "drawing_policy_registry.h"
#include "i_renderer.h"
#include "opengl_resource_manager.h"
#include "pipeline_config_registry.h"
#include "render_graph_executor.h"
#include "render_types.h"
#include <memory>

// Forward-declarations
namespace Render {
class Device;
}
namespace Core {
class SceneView;
}
namespace Render::OpenGL {
class OpenGLCommandList;
}

namespace Render::OpenGL {

// Per-frame resources for OpenGL (analogous to Vulkan)
struct OpenGLPerFrameResources {
  RID uniform_buffer;       // UBO for FrameUniforms (view-projection, camera)
  RID descriptor_set_rid;   // Descriptor set for Set 0 binding
};

// This is the concrete, API-dependent implementation of the IRenderer interface
// for OpenGL.
class OpenGLRenderer final : public IRenderer {
public:
  OpenGLRenderer();
  ~OpenGLRenderer();

  void init(ImGuiContext *ctx) override;

  void renderFrame(const Core::SceneView &view,
                   ImDrawData *ui_draw_data) override;

  void destroy() override;

  Device &getRenderDeivce() override { return *m_rhi_device; };

private:
  void createPerFrameResources();
  void updatePerFrameResources(const Core::SceneView &view);

  BackendType m_backend_type = BackendType::OpenGL;
  std::unique_ptr<Device> m_rhi_device;

  // Shared, API-agnostic systems
  std::unique_ptr<RenderGraphExecutor> m_executor;
  ImGuiContext *m_imgui_context = nullptr;
  OpenglResourceManager m_resource_manager;
  PipelineConfigRegistry m_pl_registry;
  DrawingPolicyRegistry m_dp_registry;

  // OpenGL command list for drawing
  std::unique_ptr<OpenGLCommandList> m_command_list;

  // Per-frame resources (Set 0: camera/projection)
  RID m_per_frame_ds_layout;
  std::vector<OpenGLPerFrameResources> m_per_frame_resources;
};

} // namespace Render::OpenGL
