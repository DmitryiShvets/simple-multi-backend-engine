#pragma once
#include <dxgiformat.h>
#include <wrl/client.h>

struct ID3D12Resource;

namespace ssme::d3d12 {

class Dx12Texture {
public:
    Dx12Texture(Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                DXGI_FORMAT format);

    Microsoft::WRL::ComPtr<ID3D12Resource> const& getHandle() const { return m_texture; }
    DXGI_FORMAT getFormat() const { return m_format; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
    DXGI_FORMAT m_format;
};

} // namespace ssme::d3d12
