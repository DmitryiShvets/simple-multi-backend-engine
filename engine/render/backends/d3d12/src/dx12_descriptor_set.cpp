#include "dx12_descriptor_set.h"
#include "dx12_device.h"

namespace ssme::d3d12 {
Dx12DescriptorSetLayout::Dx12DescriptorSetLayout(Dx12Device &device, const DescriptorLayout &desc)
    : m_device(device), m_layout(desc){
}

Dx12DescriptorSet::Dx12DescriptorSet(const DescriptorDesc &desc)
    : m_buffer_rids(desc.uniform_buffers) {}
} // namespace ssme::d3d12
