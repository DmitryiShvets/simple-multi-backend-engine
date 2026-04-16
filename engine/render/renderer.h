#pragma once

#include "core/gpu_types.h"
#include "rhi/render_device.h"

#include "resource_handle.h"
#include "resources/descriptor_set.h"
#include "resources/uniform_block.h"

#include <imgui/imgui.h>
#include <memory>
#include <vector>

namespace ssme {
class SceneView;
class CommandList;
class RenderItem;

struct FrameData {
  std::vector<ResourceHandle<UniformBuffer>> uniform_buffer;
  std::vector<ResourceHandle<DescriptorSet>> uniform_ds;
  ResourceHandle<DescriptorSetLayout> uniform_ds_layout;
};

// The main, top-level, API-agnostic interface for the entire rendering system.
// The Application will only interact with this interface.
class IRenderer {
public:
  virtual ~IRenderer() = default;

  // Renders a single frame based on the provided scene data.
  // This single call encapsulates all work, including acquiring the next image
  // and presenting it at the end.
  virtual void init(ImGuiContext *ctx) = 0;

  virtual void renderFrame(SceneView &view, ImDrawData *ui_draw_data) = 0;

  virtual void destroy() = 0;
  // virtual void cleanupUI() = 0;
  virtual void waitIdle() const = 0;

  virtual RenderDevice &getRenderDeivce() = 0;

  virtual GpuBackend getGpuBackend() = 0;

  virtual void setFrameResources(std::shared_ptr<FrameData> data) = 0;

  void draw(CommandList &cmd, const RenderItem &item);
};
} // namespace ssme
