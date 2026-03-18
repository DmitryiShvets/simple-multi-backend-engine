#include "resource_manager.h"
#include <cassert>

namespace ssme {

void ResourceManager::registerDevice(GpuBackend type, RenderDevice *device) {
  size_t index = static_cast<size_t>(type);
  if (m_devices.size() <= index) {
    m_devices.resize(index + 1, nullptr);
  }
  m_devices[index] = device;
}

void ResourceManager::clear() {
  for (auto &kv : m_resources) {
    auto &val = kv.second;
    for (auto &innerKv : val) {
      auto &loadedResource = innerKv.second;
      loadedResource->unload();
    }
    val.clear();
  }
  m_resources.clear();
}
} // namespace ssme
