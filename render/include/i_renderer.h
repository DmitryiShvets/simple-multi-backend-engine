#pragma once
#include "render_device.h"

#include <imgui.h>

namespace Core {
class SceneView;
}

namespace Render {
// The main, top-level, API-agnostic interface for the entire rendering system.
// The Application will only interact with this interface.
class IRenderer {
public:
  virtual ~IRenderer() = default;

  // Renders a single frame based on the provided scene data.
  // This single call encapsulates all work, including acquiring the next image
  // and presenting it at the end.
  virtual void init(ImGuiContext *ctx) = 0;

  virtual void renderFrame(const Core::SceneView &view,
                           ImDrawData *ui_draw_data) = 0;

  virtual void destroy() = 0;
  // virtual void cleanupUI() = 0;

  virtual RenderDevice &getRenderDeivce() = 0;
};
} // namespace Render
