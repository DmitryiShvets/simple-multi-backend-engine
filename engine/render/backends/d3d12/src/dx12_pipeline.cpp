#include "dx12_pipeline.h"
#include "com_exception.h"
#include "dx12_device.h"

namespace ssme::d3d12 {
Dx12Pipeline::Dx12Pipeline(
    Dx12Device &device,
    const Microsoft::WRL::ComPtr<ID3D12RootSignature> &root_signature,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC &pso_desc, RID pipeline_layout_id)
    : m_device(device), m_pipeline_layout(root_signature),
      m_pipeline_layout_id(pipeline_layout_id) {
  DX::ThrowIfFailed(m_device.getHandle()->CreateGraphicsPipelineState(
      &pso_desc, IID_PPV_ARGS(&m_pipeline)));
}
} // namespace ssme::d3d12
