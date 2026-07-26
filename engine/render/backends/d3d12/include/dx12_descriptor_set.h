#pragma once
#include "core/resource_types.h"
#include <wrl/client.h>

struct ID3D12PipelineState;

namespace ssme::d3d12 {
// Forward declaration
class Dx12Device;

class Dx12DescriptorSetLayout {
public:
  Dx12DescriptorSetLayout(Dx12Device &device, const DescriptorLayout &desc);
  // Non-copyable
  Dx12DescriptorSetLayout(const Dx12DescriptorSetLayout &) = delete;
  Dx12DescriptorSetLayout &operator=(const Dx12DescriptorSetLayout &) = delete;
  // Getters
  DescriptorLayout const &getLayout() const { return m_layout; }

private:
  Dx12Device &m_device;
  DescriptorLayout m_layout;
};

class Dx12DescriptorSet {
public:
  Dx12DescriptorSet(const DescriptorDesc &desc);

  // Non-copyable
  Dx12DescriptorSet(const Dx12DescriptorSet &) = delete;
  Dx12DescriptorSet &operator=(const Dx12DescriptorSet &) = delete;

  RID getBufferRID(size_t index) const { return m_buffer_rids[index]; }
  size_t getBufferCount() const { return m_buffer_rids.size(); }

private:
  std::vector<RID> m_buffer_rids;  // uniform buffer RIDs in binding order
};

} // namespace ssme::d3d12
