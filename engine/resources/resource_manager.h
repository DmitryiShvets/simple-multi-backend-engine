#pragma once

#include "core/gpu_types.h"
#include "resource.h"
#include "resource_handle.h"

#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>
namespace ssme {
class Resource;
class RenderDevice;
/**
 * @brief Class for managing m_resources.
 */
class ResourceManager {
public:
  /**
   * @brief Default constructor.
   */
  ResourceManager() = default;

  /**
   * @brief Virtual destructor for proper cleanup.
   */
  virtual ~ResourceManager() = default;

  /**
   * @brief Add render device destructor
   * @tparam GpuBackend The type of gpu backend.
   * @tparam RenderDevice The device  for proper creating, destroing of
   * resource.
   */
  void registerDevice(GpuBackend type, RenderDevice *device);
  /**
   * @brief Load a resource.
   * @tparam T The type of resource.
   * @tparam Args The types of arguments to pass to the resource constructor.
   * @param id The resource ID.
   * @param args The arguments to pass to the resource constructor.
   * @return A handle to the resource.
   */
  template <typename T, typename... Args>
  ResourceHandle<T> load(const std::string &uuid, Args &&...args) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");

    // Check if the resource already exists
    auto &typeResources = m_resources[T::ID];
    auto it = typeResources.find(uuid);
    if (it != typeResources.end()) {
      refCounts[T::ID][uuid].refCount++;
      return ResourceHandle<T>(uuid, this);
    }

    // Create and load the resource
    auto resource = std::make_unique<T>(uuid, std::forward<Args>(args)...);
    if (!resource->load()) {
      throw std::runtime_error("Failed to load resource: " + uuid);
    }

    // Store the resource
    typeResources[uuid] = std::move(resource);
    refCounts[T::ID][uuid].refCount = 1;
    return ResourceHandle<T>(uuid, this);
  }

  /**
   * @brief Get a resource.
   * @tparam T The type of resource.
   * @param id The resource ID.
   * @return A pointer to the resource, or nullptr if not found.
   */
  template <typename T> T *get(const std::string &id) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");

    auto typeIt = m_resources.find(T::ID);
    if (typeIt == m_resources.end()) {
      return nullptr;
    }

    auto &typeResources = typeIt->second;
    auto resourceIt = typeResources.find(id);
    if (resourceIt == typeResources.end()) {
      return nullptr;
    }

    return static_cast<T *>(resourceIt->second.get());
  }

  /**
   * @brief Check if a resource exists.
   * @tparam T The type of resource.
   * @param id The resource ID.
   * @return True if the resource exists, false otherwise.
   */
  template <typename T> bool has(const std::string &id) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");

    auto typeIt = m_resources.find(T::ID);
    if (typeIt == m_resources.end()) {
      return false;
    }

    auto &typeResources = typeIt->second;
    return typeResources.contains(id);
  }

  /**
   * @brief Unload a resource.
   * @tparam T The type of resource.
   * @param id The resource ID.
   * @return True if the resource was unloaded, false otherwise.
   */
  template <typename T> bool unload(const std::string &id) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");

    auto typeIt = m_resources.find(T::ID);
    if (typeIt == m_resources.end()) {
      return false;
    }

    auto &typeResources = typeIt->second;
    auto resourceIt = typeResources.find(id);
    if (resourceIt == typeResources.end()) {
      return false;
    }

    resourceIt->second->unload();
    typeResources.erase(resourceIt);
    return true;
  }

  // need test
  template <typename T> void release(const std::string &resourceId) {
    // Locate reference count entry for this resource
    auto it = refCounts.find(T::ID);
    if (it != refCounts.end()) {
      auto data = it->second.find(resourceId);
      if (data != it->second.end()) {
        data->second.refCount--;
      }

      // Check if resource has no remaining references
      if (data->second.refCount <= 0) {
        // Locate and unload the unreferenced resource across all type
        // containers
        for (auto &[type, typeResources] : m_resources) {
          auto resourceIt = typeResources.find(resourceId);
          if (resourceIt != typeResources.end()) {
            resourceIt->second->unload(); // Allow resource to clean up its data
            typeResources.erase(resourceIt); // Remove from cache
            break;
          }
        }
        // Clean up reference counting entry
        refCounts.erase(it);
      }
    }
  }
  /**
   * @brief Unload all resources.
   */
  void clear();

private:
  // Two-level storage system: organize by type first, then by unique identifier
  // This approach enables type-safe resource access while maintaining efficient
  // lookup
  std::unordered_map<std::type_index,
                     std::unordered_map<std::string, std::shared_ptr<Resource>>>
      m_resources;

  // Two-level reference counting system for automatic resource lifecycle
  // management First level maps resource type, second level maps resource IDs
  // to their data
  struct ResourceData {
    std::shared_ptr<Resource> resource; // The actual resource
    int refCount;                       // Reference count for this resource
  };
  std::unordered_map<std::type_index,
                     std::unordered_map<std::string, ResourceData>>
      refCounts;

  std::vector<RenderDevice *> m_devices;
};

// Implementation of ResourceHandle methods
template <typename T> ResourceHandle<T>::~ResourceHandle() {
  m_resource_manager->release<T>(m_uuid);
}

template <typename T> T *ResourceHandle<T>::get() const {
  if (!m_resource_manager)
    return nullptr;
  return m_resource_manager->get<T>(m_uuid);
}

template <typename T> bool ResourceHandle<T>::isValid() const {
  if (!m_resource_manager)
    return false;
  return m_resource_manager->has<T>(m_uuid);
}

} // namespace ssme
