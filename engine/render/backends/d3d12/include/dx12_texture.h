#pragma once
#include "core/resource_types.h"
#include <d3d12.h>
#include <dxgiformat.h>
#include <wrl/client.h>

struct ID3D12Resource;

namespace ssme::d3d12 {
// Forward declaration
class Dx12Device;

class Dx12Texture {
public:
  // Constructor for wrapping a swapchain image (does not own the image)
  Dx12Texture(Dx12Device &device, Microsoft::WRL::ComPtr<ID3D12Resource> image,
              DXGI_FORMAT format, D3D12_CPU_DESCRIPTOR_HANDLE rtv_slot);
  // Render-target owned texture: allocates resource + heaps
  explicit Dx12Texture(Dx12Device &device, const TextureDesc &desc);
  // Non-copyable
  Dx12Texture(const Dx12Texture &) = delete;
  Dx12Texture &operator=(const Dx12Texture &) = delete;
  // Getters
  Microsoft::WRL::ComPtr<ID3D12Resource> const &getHandle() const {
    return m_texture;
  }
  DXGI_FORMAT getFormat() const { return m_format; }
  D3D12_CPU_DESCRIPTOR_HANDLE const &getRtvHandle() const {
    return m_rtv_handle;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE const &getDsvHandle() const {
    return m_dsv_handle;
  }

  bool owns() const { return m_owns; }
  bool isDepth() const { return m_is_depth; }
  uint32_t getWidth() const { return m_width; }
  uint32_t getHeight() const { return m_height; }

private:
  Dx12Device &m_device;
  Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtv_heap;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsv_heap;
  D3D12_CPU_DESCRIPTOR_HANDLE m_rtv_handle;
  D3D12_CPU_DESCRIPTOR_HANDLE m_dsv_handle;
  DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
  bool m_owns = false;
  bool m_is_depth = false;
  uint32_t m_width = 0;
  uint32_t m_height = 0;
};

} // namespace ssme::d3d12
