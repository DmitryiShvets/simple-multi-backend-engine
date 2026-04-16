#pragma once
#include "core/resource_types.h"
#include "resource.h"

/**
 * @brief Texture resource
 *
 * Requirements:
 * - Format and Type: Support for various formats (RGBA8, RGB8, HDR) and types (2D, CubeMap).
 * - Filtering parameters: Stores Sampler settings (Linear, Nearest, Repeat, Clamp).
 * - Mips: Automatic mipmap generation for texture clarity at distance.
 * - Backend Unity: Same texture UUID creates VkImage in Vulkan and GLuint in OpenGL.
 */

namespace ssme {

class Texture : public Resource {
public:
  using ParamsType = TextureDesc;
  static constexpr ResourceId ID = ResourceId::TEXTURE;
  static constexpr uint32_t COMPONENTS = 1;

  Texture(std::string id, const VecRefRD &devices, const TextureDesc &desc);
  ~Texture() override;

  // Resource interface
  uint32_t doPrepare() override;
  void doSetup(const VecRID &rids) override;
  bool doLoad() override;
  bool doUnload() override;

  // Getters
  RID getTexture() const { return m_texture_id; }
  const TextureDesc &getDesc() const { return m_desc; }

private:
  TextureDesc m_desc;
  RID m_texture_id;
};

} // namespace ssme
