#include "resource_manager.h"
#include "utils/logger.h"
#include <cassert>
#include <string>

namespace ssme {

void ResourceManager::registerDevice(GpuBackend type, RenderDevice *device) {
  size_t index = static_cast<size_t>(type);
  if (m_devices.size() <= index) {
    m_devices.resize(index + 1, nullptr);
  }
  m_devices[index] = device;
  // avoid broken refs
  m_device_refs.clear();
  // Convert vector<RenderDevice*> to VecRefRD
  for (auto *device : m_devices) {
    if (device) {
      m_device_refs.emplace_back(*device);
    }
  }
}

void ResourceManager::clear() {
  // printStats();
  // Force unload all resources (regardless of refcount)
  for (uint32_t index = 0 ; index < m_slots.size(); index++) {
      unload(index, m_slots[index].generation);
  }
  // printStats();
}

void ResourceManager::printStats() const {
  size_t total_resources = 0;
  size_t total_refcount = 0;
  for (auto &slot : m_slots) {
    if (slot.ptr) {
      total_resources++;
      total_refcount += slot.ptr.get()->getUsersCount();
    }
  }

  Logger::info_log("Live resources: " + std::to_string(total_resources));
  Logger::info_log("Total refcount: " + std::to_string(total_refcount));
}

void ResourceManager::registerLoader(std::unique_ptr<IResourceLoader> loader) {
    m_loaders[loader->getResourceId()] = std::move(loader);
}
} // namespace ssme
