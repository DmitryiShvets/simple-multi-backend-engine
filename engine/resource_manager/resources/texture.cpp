#include "texture.h"
#include "render_device.h"

namespace ssme {

Texture::Texture(std::string id, const VecRefRD &devices,
                 const TextureDesc &desc)
    : Resource(id, devices), m_desc(desc) {}
Texture::~Texture() {}


uint32_t Texture::doPrepare() {
    return COMPONENTS;
}

void Texture::doSetup(const VecRID &rids) { m_texture_id = rids[0]; }

bool Texture::doLoad() {
  // 1. If path specified and no data — load file (stb_image)
  if (!m_desc.source_path.empty() && m_desc.raw_data.empty()) {
    // m_desc = ImageLoader::load(m_desc.source_path);
    // Here we fill width, height and raw_data
  }

  if (m_desc.raw_data.empty())
    return false;

  // 2. Create texture in all backends
  for (auto &rd : m_devices) {
    rd.get().createTexture(m_desc, m_texture_id);
  }

  // Clear RAM after uploading to GPU
  m_desc.raw_data.clear();
  m_desc.raw_data.shrink_to_fit();

  return true;
}

bool Texture::doUnload() {
  for (auto &rd : m_devices) {
    rd.get().destroyTexture(m_texture_id);
  }
  return true;
}

} // namespace ssme
