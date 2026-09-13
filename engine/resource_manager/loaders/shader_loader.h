#pragma once

#include "resource_loader.h"
// #include "spirv_reflect.h"

namespace ssme {

class SlangCompiler;

class ShaderLoader : public IResourceLoader {
public:
  ShaderLoader();
  ~ShaderLoader() override;

  ResourceId getResourceId() const override { return ResourceId::SHADER; }

  bool load(const std::string &path, ResourceManager &rm,
            void *out_params) override;

private:
  std::unique_ptr<SlangCompiler> m_compiler;
};
} // namespace ssme
