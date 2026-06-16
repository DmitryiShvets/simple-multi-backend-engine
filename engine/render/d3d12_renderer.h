#pragma once

#include "core/gpu_types.h"
#include "dx12_gpu_storage.h"
#include "renderer.h"
#include "dx12_command_list.h"
#include <memory>

namespace ssme {

class RenderDevice;
class Platform;
class ResourceManager;
class SceneView;

// This is the concrete, API-dependent implementation of the IRenderer interface
// for DirectX 12.
class Dx12Renderer final : public IRenderer {
public:
  Dx12Renderer(Platform *platform, ResourceManager* rm );
  ~Dx12Renderer();

  void init(ImGuiContext *ctx) override;

  void renderFrame(SceneView &view, ImDrawData *ui_draw_data) override;

  void destroy() override;

  void waitIdle() const override;

  RenderDevice &getRenderDeivce() override { return *m_rhi_device; };

  GpuBackend getGpuBackend() override;

  void setFrameResources(std::shared_ptr<FrameData> data) override;

private:
  void createPerFrameResources();
  void updatePerFrameResources(const SceneView &view);

  GpuBackend m_backend_type = GpuBackend::OpenGL;
  Platform *m_platform;
  ResourceManager* m_rm;

  std::unique_ptr<RenderDevice> m_rhi_device;

  // Shared, API-agnostic systems
  ImGuiContext *m_imgui_context = nullptr;
  ssme::d3d12::Dx12GpuStorageMT m_storage;

  // OpenGL command list for drawing
  std::unique_ptr<ssme::d3d12::Dx12CommandList> m_command_list;

  std::shared_ptr<FrameData> m_frame_data = nullptr;
};

} // namespace ssme
