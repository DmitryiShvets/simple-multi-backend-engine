#pragma once
#include <cstdint>
#include <wrl/client.h>

struct ID3D12Resource;

namespace ssme::d3d12 {

class Dx12Buffer {
public:
    Dx12Buffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource,
               uint64_t size);

    Microsoft::WRL::ComPtr<ID3D12Resource> const& getHandle() const { return m_buffer; }
    uint64_t getSize() const { return m_size; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
    uint64_t m_size;
};

} // namespace ssme::d3d12
