#pragma once
#include "core/rid.h"
#include "d3dx12.h"
#include <wrl/client.h>

struct ID3D12PipelineState;

namespace ssme::d3d12 {
// Forward declaration
class Dx12Device;

class Dx12Pipeline {
public:
  Dx12Pipeline(
      Dx12Device &device,
      const Microsoft::WRL::ComPtr<ID3D12RootSignature> &root_signature,
      const D3D12_GRAPHICS_PIPELINE_STATE_DESC &pso_desc, RID pipeline_layout_id);
  // Non-copyable
  Dx12Pipeline(const Dx12Pipeline &) = delete;
  Dx12Pipeline &operator=(const Dx12Pipeline &) = delete;
  // Getters
  Microsoft::WRL::ComPtr<ID3D12PipelineState> const &getHandle() const {
    return m_pipeline;
  }
  Microsoft::WRL::ComPtr<ID3D12RootSignature> const &getLayoutHandle() const {
    return m_pipeline_layout;
  }

  RID const& getLayoutRID() const {
      return  m_pipeline_layout_id;
  }

private:
  Dx12Device &m_device;
  Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline;
  Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pipeline_layout;
  RID m_pipeline_layout_id = RID::INVALID;
};

} // namespace ssme::d3d12
