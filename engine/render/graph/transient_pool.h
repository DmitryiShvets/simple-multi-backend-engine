#pragma once
#include "core/resource_types.h"
#include "core/rid.h"
#include "resource_handle.h"
#include "resources/texture.h"
#include <string>
#include <unordered_map>

namespace ssme {

class ResourceManager;

class TransientPool {
public:
  explicit TransientPool(ResourceManager *rm) : m_rm(rm) {}

  // Вернёт RID: из кэша (если desc совпадает) или создаст новую текстуру.
  RID getOrCreate(const std::string &name, const TextureDesc &desc);
  void clear(); // при шатдауне/ресайзе — release хэндлов → RM разгрузит

private:
  ResourceManager *m_rm;
  std::unordered_map<std::string, ResourceHandle<Texture>> m_resources;
};

} // namespace ssme
