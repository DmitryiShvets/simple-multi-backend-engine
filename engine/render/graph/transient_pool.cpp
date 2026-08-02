#include "transient_pool.h"
#include "resource_manager.h"

namespace ssme {

RID TransientPool::getOrCreate(const std::string &name, const TextureDesc &desc) {
  auto it = m_resources.find(name);
  if (it != m_resources.end()) {
    auto *tex = it->second.get();
    if (tex) {
      const auto &cached = tex->getDesc();
      if (cached.width == desc.width && cached.height == desc.height &&
          cached.format == desc.format)
        return tex->getTexture(); // переиспользуем
      m_resources.erase(it);      // desc изменился (ресайз) → пересоздаём
    }
  }
  auto handle = m_rm->load<Texture>("transient/" + name, desc);
  RID rid = handle.get()->getTexture();
  m_resources.emplace(name, std::move(handle));
  return rid;
}

void TransientPool::clear() { m_resources.clear(); }

} // namespace ssme
