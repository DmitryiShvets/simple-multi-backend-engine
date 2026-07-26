#pragma once

#include "core/gpu_types.h"
#include "dx12_command_list.h"
#include "dx12_gpu_storage.h"
#include "dx12_render_device.h"
#include "dx12_swap_chain.h"
#include "renderer.h"
#include <memory>

namespace ssme::d3d12 {
class Dx12Device;
}

namespace ssme {

class RenderDevice;
class Platform;
class ResourceManager;
class SceneView;

// This is the concrete, API-dependent implementation of the IRenderer interface
// for DirectX 12.
class Dx12Renderer final : public IRenderer {
public:
  Dx12Renderer(Platform *platform, ResourceManager *rm);
  ~Dx12Renderer();

  void init(ImGuiContext *ctx) override;

  void renderFrame(SceneView &view, ImDrawData *ui_draw_data) override;

  void destroy() override;

  void waitIdle() const override;

  RenderDevice &getRenderDeivce() override { return *m_rhi_device; };

  GpuBackend getGpuBackend() override;

  void setFrameResources(std::shared_ptr<FrameData> data) override;

  struct ImGuiSrvHandle {
      D3D12_CPU_DESCRIPTOR_HANDLE cpu = {};
      D3D12_GPU_DESCRIPTOR_HANDLE gpu = {};
      bool allocated = false;
  };
  ImGuiSrvHandle m_imgui_srv_handle;

private:
  GpuBackend m_backend_type = GpuBackend::DirectX12;
  Platform *m_platform;
  ResourceManager *m_rm;
  std::unique_ptr<ssme::d3d12::Dx12Device> m_device;
  std::unique_ptr<ssme::d3d12::Dx12RenderDevice> m_rhi_device;
  ssme::d3d12::Dx12GpuStorageMT m_storage;
  // --- Frame and Swapchain Management ---
  std::unique_ptr<ssme::d3d12::Dx12SwapChain> m_swap_chain;
  std::vector<std::unique_ptr<ssme::d3d12::Dx12CommandList>> m_command_lists;
  uint32_t m_acquired_image_index = 0;
  // --- Rendering Logic (Orchestration) ---
  ImGuiContext *m_imgui_context = nullptr;

  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_imgui_srv_heap;
  bool m_imgui_initialized = false;

  std::shared_ptr<FrameData> m_frame_data = nullptr;
  // Private, API-dependent methods for frame lifecycle management

  void updatePerFrameResources(const SceneView &view);
};

} // namespace ssme
