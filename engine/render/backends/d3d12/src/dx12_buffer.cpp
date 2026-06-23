#include "dx12_buffer.h"
#include <d3d12.h>

namespace ssme::d3d12 {

Dx12Buffer::Dx12Buffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                       uint64_t size)
    : m_buffer(resource), m_size(size) {}

} // namespace ssme::d3d12
