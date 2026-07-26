#include "dx12_buffer.h"
#include "com_exception.h"
#include "d3dx12.h"
#include "dx12_device.h"
#include <d3d12.h>
namespace ssme::d3d12 {
Dx12Buffer::Dx12Buffer(Dx12Device &device, uint64_t instanceSize,
                       uint64_t instanceCount, D3D12_HEAP_TYPE heapType,
                       D3D12_RESOURCE_STATES initialState,
                       uint64_t minOffsetAlignment)
    : m_device(device), m_instance_size(instanceSize),
      m_instance_count(instanceCount) {
  m_alignment_size = getAlignment(instanceSize, minOffsetAlignment);
  m_buffer_size = m_alignment_size * m_instance_count;

  CD3DX12_HEAP_PROPERTIES heapProps(heapType);
  CD3DX12_RESOURCE_DESC bufferDesc =
      CD3DX12_RESOURCE_DESC::Buffer(m_buffer_size);
  DX::ThrowIfFailed(m_device.getHandle()->CreateCommittedResource(
      &heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
      initialState, nullptr, IID_PPV_ARGS(&m_buffer)));
}

Dx12Buffer::~Dx12Buffer() {
  if (m_mapped)
    unmap();
}

uint64_t Dx12Buffer::getAlignment(uint64_t instanceSize,
                                  uint64_t minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}

void Dx12Buffer::map(uint64_t size, uint64_t offset) {
  D3D12_RANGE range = {offset, size ? offset + size : m_buffer_size};
  DX::ThrowIfFailed(m_buffer->Map(0, &range, &m_mapped));
}

void Dx12Buffer::unmap() {
  m_buffer->Unmap(0, nullptr);
  m_mapped = nullptr;
}

void Dx12Buffer::writeToBuffer(const void *data, uint64_t size,
                               uint64_t offset) {
  if (!m_mapped)
    return;
  auto *ptr = static_cast<char *>(m_mapped) + offset;
  std::memcpy(ptr, data, size ? size : m_buffer_size);
}

void Dx12Buffer::writeToIndex(const void *data, uint64_t index) {
  writeToBuffer(data, m_instance_size, index * m_alignment_size);
}

D3D12_GPU_VIRTUAL_ADDRESS Dx12Buffer::getAddress() const {
  return m_buffer->GetGPUVirtualAddress();
}
D3D12_GPU_VIRTUAL_ADDRESS Dx12Buffer::getAddressForIndex(uint64_t index) const {
  return m_buffer->GetGPUVirtualAddress() + index * m_alignment_size;
}

} // namespace ssme::d3d12
