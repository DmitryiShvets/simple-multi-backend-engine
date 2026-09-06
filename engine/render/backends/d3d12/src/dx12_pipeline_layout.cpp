#include "dx12_pipeline_layout.h"
#include "com_exception.h"
#include "dx12_device.h"
#include <cassert>
#include <vector>

namespace ssme::d3d12 {

Dx12PipelineLayout::Dx12PipelineLayout(
    Dx12Device &device, std::vector<CD3DX12_ROOT_PARAMETER> params,
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges,
    uint32_t push_constant_start_index)
    : m_device(device) {

  D3D12_STATIC_SAMPLER_DESC static_sampler = {};
  static_sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
  static_sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  static_sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  static_sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  static_sampler.MipLODBias = 0;
  static_sampler.MaxAnisotropy = 0;
  static_sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
  static_sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
  static_sampler.MinLOD = 0.0f;
  static_sampler.MaxLOD = D3D12_FLOAT32_MAX;
  static_sampler.ShaderRegister = 0;
  static_sampler.RegisterSpace = 3;
  static_sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

  CD3DX12_ROOT_SIGNATURE_DESC desc(
      (UINT)params.size(), params.data(), 1, &static_sampler,
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
  DX::ThrowIfFailed(D3D12SerializeRootSignature(
      &desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
  DX::ThrowIfFailed(m_device.getHandle()->CreateRootSignature(
      0, signature->GetBufferPointer(), signature->GetBufferSize(),
      IID_PPV_ARGS(&m_root_signature)));
  assert((UINT)params.size() > push_constant_start_index);
  for (uint32_t i = 0; i < push_constant_start_index; i++) {
    const auto &param = params[i];
    uint32_t set_index;
    if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
      set_index = param.DescriptorTable.pDescriptorRanges[0].RegisterSpace;
    } else {
      set_index = param.Descriptor.RegisterSpace;
    }
    m_root_params_map[set_index].push_back({i, param.ParameterType});
  }

  m_push_constant_param_index = push_constant_start_index;
}
} // namespace ssme::d3d12
