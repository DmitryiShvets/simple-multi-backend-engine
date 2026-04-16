#pragma once

#include "resource_loader.h"
#include "spirv_reflect.h"

namespace ssme {

UniformValue::Type toUniformValueType(SpvReflectTypeDescription *type_desc);
DescriptorType toDescriptorType(SpvReflectDescriptorType type);
Format toVertexFormat(SpvReflectFormat format);

class ShaderLoader : public IResourceLoader {
public:
  ResourceId getResourceId() const override { return ResourceId::SHADER; }

  bool load(const std::string &path, ResourceManager &rm,
            void *out_params) override;
};
} // namespace ssme
