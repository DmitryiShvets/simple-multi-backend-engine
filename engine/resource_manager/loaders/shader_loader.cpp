#include "shader_loader.h"
#include "core/render_types.h"
#include "slang_compiler.h"
#include "utils/debug_assert.h"

namespace ssme {

ShaderLoader::ShaderLoader() : m_compiler(std::make_unique<SlangCompiler>()) {
  debug_assert(m_compiler->init(), "SlangCompiler init failed");
}

ShaderLoader::~ShaderLoader() = default;

bool ShaderLoader::load(const std::string &path, ResourceManager &rm,
                        void *out_params) {
  (void)rm;
  auto *desc = static_cast<ShaderModuleDesc *>(out_params);

  ShaderStage stage = ShaderStage::VERTEX;
  if (path[0] == 'v') {
    stage = ShaderStage::VERTEX;
  } else if (path[0] == 'f') {
    stage = ShaderStage::FRAGMENT;
  } else {
    debug_assert(false, "Unknown shader stage prefix in resource: " + path);
    return false;
  }

  return m_compiler->compile(stage, path, *desc);
}

} // namespace ssme
