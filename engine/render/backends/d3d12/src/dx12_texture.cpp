#include "dx12_texture.h"
#include "dx12_device.h"
#include <d3d12.h>

namespace ssme::d3d12 {

Dx12Texture::Dx12Texture(Dx12Device &device,
                         Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                         DXGI_FORMAT format,
                         D3D12_CPU_DESCRIPTOR_HANDLE rtv_slot)
    : m_device(device), m_texture(resource), m_rtv_handle(rtv_slot),
      m_format(format) {}

} // namespace ssme::d3d12
