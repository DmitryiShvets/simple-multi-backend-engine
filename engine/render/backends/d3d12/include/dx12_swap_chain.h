#pragma once

#include "core/render_types.h"
#include "core/rid.h"
#include "dx12_gpu_storage_fwd.h"
#include <vector>
#include <wrl/client.h>
#include <dxgi1_6.h>
// Forward declaration
struct IDXGISwapChain3;
struct ID3D12CommandQueue;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12Fence;
struct CD3DX12_CPU_DESCRIPTOR_HANDLE;

namespace ssme {
class Platform;
}
namespace ssme::d3d12 {
// Forward declaration
class Dx12Device;

class Dx12SwapChain {
public:
  /**
   * @brief Constructs a new swap chain.
   * @param device A reference to the Dx12Device.
   * @param windowExtent The width and height of the window.
   */
  Dx12SwapChain(Dx12Device &device, Extent2D windowExtent,
                Dx12GpuStorageMT &resourceManager, ssme::Platform *platform);

  /**
   * @brief Acquires the index of the next available image from the swap chain
   * to be rendered into.
   * @return A UINT that will be filled with the
   * acquired image index.
   */
  void acquireNextImage(); // wait for fences and retrives new image

  /**
   * @brief Submits the provided command buffers for execution and presents the
   * rendered image to the screen.
   * @param buffers A pointer to an array of command lists to be submitted.
   * rendered into.
   * date.
   */
  void submitCommandBuffers(
      const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> &commandList);

  // --- Getters ---
  size_t getCurrentFrameIndex() const { return m_frame_index; }
  size_t getImageCount() { return m_swap_chain_images.size(); }
  RID getTextureRID(uint32_t index) const;

  Microsoft::WRL::ComPtr<IDXGISwapChain3> const &getHandle() const & {
    return m_swap_chain;
  }

  CD3DX12_CPU_DESCRIPTOR_HANDLE getRtvHandle(UINT frame_index);

private:
  /** @brief Main initialization function that calls all the creation helpers.
   */
  void init();
  /** @brief Creates the core `IDXGISwapChain3` object. */
  void createSwapChain();
  /** @brief Creates a VulkanTexture wrapper for each VkImage in the swap chain.
   */
  void createTextureWrappers();
  /** @brief Creates synchronization objects (semaphores and fences) needed for
   * the frame loop. */
  void createSyncObjects();

  Dx12Device &m_device;
  Dx12GpuStorageMT &m_rm;
  ssme::Platform *m_platform;
  Extent2D m_extent;
  // UINT m_frameIndex;
  Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swap_chain = nullptr;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptor_heap = nullptr;
  std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_swap_chain_images;

  UINT m_rtv_descriptor_size;
  // --- DirectX Object Handles ---
  std::vector<RID> m_swap_chain_texture_rids;
  // --- Synchronization Objects ---
  Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
  HANDLE m_fence_event = nullptr;
  UINT64 m_fence_value = 0;
  UINT m_frame_index = 0;
};
} // namespace ssme::d3d12
