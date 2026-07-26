#include "dx12_pipeline_layout.h"
#include "com_exception.h"
#include "dx12_device.h"
#include <cassert>
#include <vector>

namespace ssme::d3d12 {
Dx12PipelineLayout::Dx12PipelineLayout(
    Dx12Device &device, const std::vector<CD3DX12_ROOT_PARAMETER> &params, uint32_t push_constant_start_index)
    : m_device(device) {
  CD3DX12_ROOT_SIGNATURE_DESC desc(
      (UINT)params.size(), params.data(), 0, nullptr,
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
  DX::ThrowIfFailed(D3D12SerializeRootSignature(
      &desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
  DX::ThrowIfFailed(m_device.getHandle()->CreateRootSignature(
      0, signature->GetBufferPointer(), signature->GetBufferSize(),
      IID_PPV_ARGS(&m_root_signature)));
  assert((UINT)params.size() > push_constant_start_index);
  for (uint32_t i = 0; i < push_constant_start_index; i++) {
      const auto param = params[i];
      const auto set_index = param.Descriptor.RegisterSpace;
      const auto set_values = m_root_params_map.find(set_index);
      if(set_values == m_root_params_map.end()) {
          m_root_params_map[set_index] = {};
      }
       m_root_params_map.at(set_index).push_back(i);
  }
  m_push_constant_param_index = push_constant_start_index;
}
} // namespace ssme::d3d12
