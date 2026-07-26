#pragma once
#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>

namespace ssme::d3d12 {
class Dx12Device;

class Dx12Buffer {
public:
  Dx12Buffer(Dx12Device &device, uint64_t instanceSize, uint64_t instanceCount,
             D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState,
             uint64_t minOffsetAlignment = 1);
  ~Dx12Buffer();
  void map(uint64_t size = 0, uint64_t offset = 0);
  void unmap();
  void writeToBuffer(const void *data, uint64_t size = 0, uint64_t offset = 0);
  void writeToIndex(const void *data, uint64_t index);
  // Non-copyable
  Dx12Buffer(const Dx12Buffer &) = delete;
  Dx12Buffer &operator=(const Dx12Buffer &) = delete;
  // Getters
  Microsoft::WRL::ComPtr<ID3D12Resource> const &getHandle() const {
    return m_buffer;
  }
  D3D12_GPU_VIRTUAL_ADDRESS getAddress() const;
  D3D12_GPU_VIRTUAL_ADDRESS getAddressForIndex(uint64_t index) const;
  uint64_t getBufferSize() const { return m_buffer_size; }
  uint64_t getInstanceSize() const { return m_instance_size; }
  uint64_t getAlignmentSize() const { return m_alignment_size; }
  uint64_t getInstanceCount() const { return m_instance_count; }
  void *getMappedMemory() const { return m_mapped; }

private:
  static uint64_t getAlignment(uint64_t instanceSize,
                               uint64_t minOffsetAlignment);

  Dx12Device &m_device;
  Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
  void *m_mapped = nullptr;

  uint64_t m_buffer_size; // total buffer size = alignment_size * instance_count
  uint64_t m_instance_size; // stride
  uint64_t m_alignment_size;
  uint64_t m_instance_count;
};
} // namespace ssme::d3d12
