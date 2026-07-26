#pragma once
#include "d3dx12.h"
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <wrl/client.h>

struct ID3D12RootSignature;

namespace ssme::d3d12 {
// Forward declaration
class Dx12Device;

class Dx12PipelineLayout {
public:
  Dx12PipelineLayout(Dx12Device &device, const std::vector<CD3DX12_ROOT_PARAMETER> &params, uint32_t push_constant_start_index);

  // Non-copyable
  Dx12PipelineLayout(const Dx12PipelineLayout &) = delete;
  Dx12PipelineLayout &operator=(const Dx12PipelineLayout &) = delete;
  // Getters
  Microsoft::WRL::ComPtr<ID3D12RootSignature> const &getHandle() const {
    return m_root_signature;
  }

  std::vector<uint32_t> const & getParamIndices(uint32_t index) const {
      return m_root_params_map.at(index);
  }

  uint32_t getPushConstantParamIndex() const { return m_push_constant_param_index; }
  bool hasPushConstant() const { return m_push_constant_param_index != UINT32_MAX; }
private:
  Dx12Device &m_device;
  Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature;
  std::unordered_map<uint32_t, std::vector<uint32_t>> m_root_params_map;
  uint32_t m_push_constant_param_index = UINT32_MAX;

};

} // namespace ssme::d3d12
