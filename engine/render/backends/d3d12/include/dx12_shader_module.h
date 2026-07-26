#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <string>

namespace ssme::d3d12 {
// Forward declaration
class Dx12Device;

class Dx12ShaderModule {
public:
  Dx12ShaderModule(Dx12Device &device, const std::string & shader_filepath);
  // Non-copyable
  Dx12ShaderModule(const Dx12ShaderModule &) = delete;
  Dx12ShaderModule &operator=(const Dx12ShaderModule &) = delete;
  // Getters
  Microsoft::WRL::ComPtr<ID3DBlob> const &getHandle() const {
    return m_blob;
  }
  D3D12_SHADER_BYTECODE getBytecode() const { return m_bytecode; }

private:
  Dx12Device &m_device;
  Microsoft::WRL::ComPtr<ID3DBlob> m_blob;
  D3D12_SHADER_BYTECODE m_bytecode{};
};

} // namespace ssme::d3d12
