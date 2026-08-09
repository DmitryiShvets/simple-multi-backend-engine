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
struct RenderItem;
class RenderGraph;
struct CompiledPlan;


struct FrameData {
  std::vector<ResourceHandle<UniformBuffer>> uniform_buffer;
  std::vector<ResourceHandle<DescriptorSet>> uniform_ds;
  ResourceHandle<DescriptorSetLayout> uniform_ds_layout;
};

struct SwapchainInfo {
  RID color;         // RID текущего кадра свапчейна
  Extent2D extent;
};

// The main, top-level, API-agnostic interface for the entire rendering system.
// The Application will only interact with this interface.
class IRenderer {
public:
  virtual ~IRenderer() = default;
  virtual void init(ImGuiContext *ctx) = 0;
  virtual void destroy() = 0;
  virtual void waitIdle() const = 0;
  virtual RenderDevice &getRenderDeivce() = 0;
  virtual GpuBackend getGpuBackend() = 0;
  virtual void setFrameResources(std::shared_ptr<FrameData> data) = 0;

  // ── Frame-graph flow (Vulkan-first) ──
  virtual bool supportsFrameGraph() const { return false; }
  virtual void beginFrame() {}                          // Vulkan: acquire
  virtual SwapchainInfo getSwapchain() = 0;             // после beginFrame
  virtual uint32_t getCurrentFrameIndex() const = 0;
  virtual void renderImGui(CommandList &cmd, ImDrawData *ui) = 0;
  virtual void renderFrameGraph(RenderGraph &graph, const CompiledPlan &plan,
                                SceneView &view, ImDrawData *ui) = 0;

  // ── Legacy path (GL пока не мигрирован) ──
  virtual void renderFrame(SceneView &view, ImDrawData *ui) {}

  void draw(CommandList &cmd, const RenderItem &item);
};
} // namespace ssme
