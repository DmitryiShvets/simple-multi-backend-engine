#pragma once
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

private:
  Dx12Device &m_device;
  Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
  D3D12_CPU_DESCRIPTOR_HANDLE m_rtv_handle;
  DXGI_FORMAT m_format;
};

} // namespace ssme::d3d12
