#include "dx12_texture.h"
#include <d3d12.h>

namespace ssme::d3d12 {

Dx12Texture::Dx12Texture(Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                         DXGI_FORMAT format)
    : m_texture(resource), m_format(format) {}

} // namespace ssme::d3d12
