#include "dx12_shader_module.h"
#include "com_exception.h"
#include <d3dcompiler.h>
#include <vector>

namespace ssme::d3d12 {
static std::wstring toWide(const std::string &s) {
  int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
  std::wstring wstr(len, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wstr.data(), len);
  return wstr;
}

Dx12ShaderModule::Dx12ShaderModule(Dx12Device &device,
                                   const std::string &shader_filepath)
    : m_device(device) {
  DX::ThrowIfFailed(
      D3DReadFileToBlob(toWide(shader_filepath).c_str(), &m_blob));
  m_bytecode = D3D12_SHADER_BYTECODE{
      .pShaderBytecode = m_blob->GetBufferPointer(),
      .BytecodeLength = m_blob->GetBufferSize(),
  };
}

Dx12ShaderModule::Dx12ShaderModule(Dx12Device &device, const std::vector<char> &code)
    : m_device(device) {
  DX::ThrowIfFailed(D3DCreateBlob(code.size(), &m_blob));
  memcpy(m_blob->GetBufferPointer(), code.data(), code.size());
  m_bytecode = D3D12_SHADER_BYTECODE{
      .pShaderBytecode = m_blob->GetBufferPointer(),
      .BytecodeLength = m_blob->GetBufferSize(),
  };
}

} // namespace ssme::d3d12
