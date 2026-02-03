#pragma once

#include <memory>
#include <unordered_map>

#include "render_types.h"

namespace Render {

template <typename T> class ResourceOwner {
public:
  void insert(RID rid, std::unique_ptr<T> resource) {
    m_resources[rid] = std::move(resource);
  }

  T *get(RID rid) {
    auto it = m_resources.find(rid);
    if (it != m_resources.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  std::unique_ptr<T> remove(RID rid) {
    auto it = m_resources.find(rid);
    if (it != m_resources.end()) {
      auto resource = std::move(it->second);
      m_resources.erase(it);
      return resource;
    }
    return nullptr;
  }

private:
  std::unordered_map<RID, std::unique_ptr<T>> m_resources;
};

// A registry for non-owned resources, like native handles.
// It stores resources by value.
template <typename T> class ResourceRegistry {
public:
  void insert(RID rid, T resource) { m_resources[rid] = resource; }

  T get(RID rid) {
    auto it = m_resources.find(rid);
    if (it != m_resources.end()) {
      return it->second;
    }
    return nullptr;
  }

  void remove(RID rid) { m_resources.erase(rid); }

private:
  std::unordered_map<RID, T> m_resources;
};

} // namespace Render
