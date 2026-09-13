#pragma once

// Uncomment to dump the compiled shader artifacts (GLSL / SPIR-V / DXIL and a
// reflection summary) to res/shaders/dump/ for debugging.
#define SSME_DUMP_SLANG_OUTPUT

#include <slang.h>
#include <slang-com-ptr.h>

#include <string>

#include "core/resource_types.h"

namespace ssme {

class SlangCompiler {
public:
  SlangCompiler() = default;
  SlangCompiler(const SlangCompiler&) = delete;
  SlangCompiler& operator=(const SlangCompiler&) = delete;

  bool init();
  bool compile(ShaderStage stage, const std::string& name, ShaderModuleDesc& out);

private:
  bool buildReflection(slang::IComponentType* program, ShaderStage stage, ShaderModuleDesc& out);
  void logDiagnostics(const std::string& what, slang::IBlob* diag) const;
  bool dxilAvailable();
  #ifdef SSME_DUMP_SLANG_OUTPUT
    void dumpArtifacts(const ShaderModuleDesc& out) const;
  #endif


  static std::shared_ptr<UniformLayout> buildStructLayout(slang::TypeLayoutReflection* structTl);
  static Format toVertexFormat(slang::TypeLayoutReflection* typeLayout);
  static bool hasCategory(slang::VariableLayoutReflection* varLayout,
                          slang::ParameterCategory category);
  static UniformValue::Type toUniformType(slang::TypeLayoutReflection* typeLayout);

  Slang::ComPtr<slang::IGlobalSession> m_global_session;
  Slang::ComPtr<slang::ISession> m_session;
  int m_target_count = 0;
  SlangCompileTarget m_target_formats[4];
};

} // namespace ssme
