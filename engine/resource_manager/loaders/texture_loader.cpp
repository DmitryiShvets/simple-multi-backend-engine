#include "texture_loader.h"
#include "core/resource_types.h"

namespace ssme {
bool TextureLoader::load(const std::string &path, ResourceManager &rm,
                         void *out_params) {
  auto *desc = static_cast<TextureDesc *>(out_params);
  desc->source_path = path;
  desc->format = Format::R8G8B8A8_SRGB;
  desc->usage = ImageUsage::SHADER_READ;
  desc->generate_mips = true;
  return true;
}
} // namespace ssme
