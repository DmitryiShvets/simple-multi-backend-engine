#pragma once
#include "resource_loader.h"

namespace ssme {
class TextureLoader : public IResourceLoader {
public:
  ResourceId getResourceId() const override { return ResourceId::TEXTURE; }
  bool load(const std::string &path, ResourceManager &rm,
            void *out_params) override;
};
} // namespace ssme
