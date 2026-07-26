#include "dx12_swap_chain.h"
#include "com_exception.h"
#include "core/gpu_types.h"
#include "d3dx12.h"
#include "dx12_device.h"
#include "dx12_gpu_storage.h"
#include "main_window.h"
#include "platform.h"
#include <d3d12.h>
#include <dxgi1_6.h>

namespace ssme::d3d12 {

Dx12SwapChain::Dx12SwapChain(Dx12Device &device, Extent2D windowExtent,
                             Dx12GpuStorageMT &resourceManager,
                             ssme::Platform *platform)
    : m_device(device), m_rm(resourceManager), m_platform(platform),
      m_extent(windowExtent) {
  init();
}

void Dx12SwapChain::init() {
  createSwapChain();
  createTextureWrappers();
  createSyncObjects();
}

void Dx12SwapChain::acquireNextImage() {
  // Ждём fence предыдущего фрейма
  if (m_fence->GetCompletedValue() < m_fence_value) {
    DX::ThrowIfFailed(
        m_fence->SetEventOnCompletion(m_fence_value, m_fence_event));
    WaitForSingleObject(m_fence_event, INFINITE);
  }
  // Можно обновить frame_index из swap chain
  m_frame_index = m_swap_chain->GetCurrentBackBufferIndex();
}

void Dx12SwapChain::submitCommandBuffers(
    const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> &commandList) {
  // Submit
  ID3D12CommandList *ppLists[] = {commandList.Get()};
  m_device.getCommandQueue()->ExecuteCommandLists(_countof(ppLists), ppLists);

  // Present
  m_swap_chain->Present(1, 0);

  // Signal fence
  m_fence_value++;
  m_device.getCommandQueue()->Signal(m_fence.Get(), m_fence_value);

  // Обновить frame_index
  m_frame_index = m_swap_chain->GetCurrentBackBufferIndex();
}

void Dx12SwapChain::createSwapChain() {
  auto FRAMES_IN_FLIGHT = MAX_FRAMES_IN_FLIGHT;
  // Describe and create the swap chain.
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.BufferCount = FRAMES_IN_FLIGHT;
  swapChainDesc.Width = m_extent.width;
  swapChainDesc.Height = m_extent.height;
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.SampleDesc.Count = 1;

  auto &dx_window =
      m_platform->getWindow(GpuBackend::DirectX12);
  HWND hwnd = (HWND)dx_window.getNativeHwnd();
  Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
  auto d = m_device.getFactory();
  // Swap chain needs the queue so that it  can force a flush on it.
  DX::ThrowIfFailed(d->CreateSwapChainForHwnd(m_device.getCommandQueue().Get(),
                                              hwnd, &swapChainDesc, nullptr,
                                              nullptr, &swapChain));
  DX::ThrowIfFailed(swapChain.As(&m_swap_chain));

  // Describe and create a render target view (RTV) descriptor heap.
  {
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FRAMES_IN_FLIGHT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    // Create descriptor heaps.
    DX::ThrowIfFailed(m_device.getHandle()->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(&m_descriptor_heap)));
    m_rtv_descriptor_size =
        m_device.getHandle()->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }
  // Create frame resources.
  {
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each frame.
    m_swap_chain_images.resize(FRAMES_IN_FLIGHT);
    for (UINT n = 0; n < FRAMES_IN_FLIGHT; n++) {
      DX::ThrowIfFailed(
          m_swap_chain->GetBuffer(n, IID_PPV_ARGS(&m_swap_chain_images[n])));
      D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
      rtv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
      m_device.getHandle()->CreateRenderTargetView(m_swap_chain_images[n].Get(),
                                                   &rtv_desc, rtvHandle);
      std::wstring bufferName = L"BackBuffer_" + std::to_wstring(n);
      m_swap_chain_images[n]->SetName(bufferName.c_str());
      rtvHandle.Offset(1, m_rtv_descriptor_size);
    }
  }
}

void Dx12SwapChain::createTextureWrappers() {
  auto FRAMES_IN_FLIGHT = getImageCount();
  m_swap_chain_texture_rids.resize(FRAMES_IN_FLIGHT);
  for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
    auto texture = std::make_unique<Dx12Texture>(
        m_device, m_swap_chain_images[i], DXGI_FORMAT_R8G8B8A8_UNORM,
        getRtvHandle(i));
    m_swap_chain_texture_rids[i] = m_rm.add(std::move(texture));
  }
}

void Dx12SwapChain::createSyncObjects() {
  // Create synchronization objects.
  DX::ThrowIfFailed(m_device.getHandle()->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                                      IID_PPV_ARGS(&m_fence)));
  // m_fence_value = 1;
  // Create an event handle to use for frame synchronization.
  m_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (m_fence_event == nullptr) {
    DX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
  }
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Dx12SwapChain::getRtvHandle(UINT frame_index) {
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(
      m_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), frame_index,
      m_rtv_descriptor_size);
  return rtv_handle;
}

RID Dx12SwapChain::getTextureRID(uint32_t index) const {
  return m_swap_chain_texture_rids[index];
}

void Dx12SwapChain::waitForGpu() {
    m_fence_value++;
    DX::ThrowIfFailed(
        m_device.getCommandQueue()->Signal(m_fence.Get(), m_fence_value));
    if (m_fence->GetCompletedValue() < m_fence_value) {
        DX::ThrowIfFailed(
            m_fence->SetEventOnCompletion(m_fence_value, m_fence_event));
        WaitForSingleObject(m_fence_event, INFINITE);
    }
}

} // namespace ssme::d3d12
