#include "dx12_descriptor_set.h"
#include "dx12_device.h"
#include "com_exception.h"

namespace ssme::d3d12 {
Dx12DescriptorSetLayout::Dx12DescriptorSetLayout(Dx12Device &device,
                                                 const DescriptorLayout &desc)
    : m_device(device), m_layout(desc) {}

Dx12DescriptorSet::Dx12DescriptorSet(
    Dx12Device &device, const DescriptorDesc &desc,
    const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> &texture_srvs)
    : m_buffer_rids(desc.uniform_buffers) {
  if (texture_srvs.empty())
    return;

  D3D12_DESCRIPTOR_HEAP_DESC hd{};
  hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  hd.NumDescriptors = (UINT)texture_srvs.size();
  hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  DX::ThrowIfFailed(
      device.getHandle()->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&m_srv_heap)));

  D3D12_CPU_DESCRIPTOR_HANDLE dst =
      m_srv_heap->GetCPUDescriptorHandleForHeapStart();
  UINT dst_range_size = (UINT)texture_srvs.size();
  device.getHandle()->CopyDescriptors(
      1, &dst, &dst_range_size, (UINT)texture_srvs.size(), texture_srvs.data(),
      nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  D3D12_GPU_DESCRIPTOR_HANDLE gpu =
      m_srv_heap->GetGPUDescriptorHandleForHeapStart();
  UINT inc = device.getHandle()->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  for (size_t i = 0; i < texture_srvs.size(); ++i) {
    m_srv_gpu_handles.push_back(gpu);
    gpu.ptr += inc;
  }
}

} // namespace ssme::d3d12
