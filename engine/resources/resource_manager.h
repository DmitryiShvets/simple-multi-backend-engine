#pragma once

#include "resource.h"
#include "resource_handle.h"

#include "core/gpu_types.h"
#include "core/resource_types.h"
#include "core/rid_allocator.h"

#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ssme {

class Resource;
class RenderDevice;

/**
 * @brief Resource data for reference counting
 */
struct ResourceData {
  std::shared_ptr<Resource> resource; // The actual resource
  int refCount = 0;                   // Reference count
};

/**
 * @brief Centralized resource manager with unified RID system
 *
 * Generates ONE RID per resource that is shared across all backends
 * (Vulkan, OpenGL, etc.). Each backend stores its GPU-specific resource
 * under the same RID.
 *
 * Architecture:
 *   ResourceManager (central)
 *     ├── RIDAllocator → generates RID 1, 2, 3...
 *     ├── m_resources → Resource* by UUID (high-level)
 *     └── m_gpu_resources → per-backend GPU resources by RID
 *
 * Usage:
 *   RID rid = resourceManager.create<Buffer>(desc);  // One RID for all
 */
class ResourceManager {
public:
  ResourceManager() = default;
  virtual ~ResourceManager() = default;

  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;

  /**
   * @brief Register a render device for a specific backend
   * @param type Backend type (Vulkan, OpenGL, etc.)
   * @param device Pointer to the render device
   */
  void registerDevice(GpuBackend type, RenderDevice *device);

  /**
   * @brief Create a new resource with unified RID
   *
   * Creates the resource and allocates ONE RID that is shared across
   * all backends. The resource is stored by UUID for high-level access,
   * and by RID for GPU-level access.
   *
   * @tparam T Resource type (must derive from Resource)
   * @tparam Args Constructor argument types
   * @param uuid Unique identifier for the resource (string-based)
   * @param args Constructor arguments
   * @return ResourceHandle<T> for automatic lifetime management
   */
  template <typename T, typename... Args>
  ResourceHandle<T> load(const std::string &uuid, Args &&...args) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");

    // Check if resource already exists by UUID
    auto &typeResources = m_resources[T::ID];
    auto it = typeResources.find(uuid);
    if (it != typeResources.end()) {
      refCounts[T::ID][uuid].refCount++;
      return ResourceHandle<T>(uuid, this);
    }

    // Allocate unified RID for all backends
    RID rid = m_rid_allocator.allocate();

    // Create and load the resource
    auto resource = std::make_unique<T>(uuid, std::forward<Args>(args)...);

    // Store RID in the resource (if it has one)
    // resource->setRid(rid);  // Optional: if Resource has RID field

    if (!resource->load()) {
      m_rid_allocator.free(rid); // Free RID on failure
      throw std::runtime_error("Failed to load resource: " + uuid);
    }

    // Store by UUID for high-level access
    typeResources[uuid] = std::move(resource);
    refCounts[T::ID][uuid].refCount = 1;

    // Store RID → UUID mapping for GPU-level access
    m_rid_to_uuid[rid.id] = {T::ID, uuid};

    return ResourceHandle<T>(uuid, this);
  }

  /**
   * @brief Get a resource by UUID (high-level access)
   * @tparam T Resource type
   * @param uuid Resource UUID
   * @return Pointer to resource, or nullptr if not found
   */
  template <typename T> T *get(const std::string &uuid) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");

    auto typeIt = m_resources.find(T::ID);
    if (typeIt == m_resources.end()) {
      return nullptr;
    }

    auto &typeResources = typeIt->second;
    auto resourceIt = typeResources.find(uuid);
    if (resourceIt == typeResources.end()) {
      return nullptr;
    }

    return static_cast<T *>(resourceIt->second.get());
  }

  /**
   * @brief Get a resource by RID (GPU-level access)
   * @tparam T Resource type
   * @param rid Resource ID (unified across backends)
   * @return Pointer to resource, or nullptr if not found
   */
  template <typename T> T *get(RID rid) {
    // Find UUID by RID
    auto mapIt = m_rid_to_uuid.find(rid.id);
    if (mapIt == m_rid_to_uuid.end()) {
      return nullptr;
    }

    const auto &[type_id, uuid] = mapIt->second;

    // Verify type matches
    if (type_id != T::ID) {
      return nullptr; // Wrong type
    }

    // Get by UUID
    return get<T>(uuid);
  }

  /**
   * @brief Check if a resource exists by UUID
   * @tparam T Resource type
   * @param uuid Resource UUID
   * @return true if exists
   */
  template <typename T> bool has(const std::string &uuid) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");

    auto typeIt = m_resources.find(T::ID);
    if (typeIt == m_resources.end()) {
      return false;
    }

    auto &typeResources = typeIt->second;
    return typeResources.contains(uuid);
  }

  /**
   * @brief Check if a resource exists by RID
   * @tparam T Resource type
   * @param rid Resource ID
   * @return true if exists and type matches
   */
  template <typename T> bool has(RID rid) {
    auto mapIt = m_rid_to_uuid.find(rid.id);
    if (mapIt == m_rid_to_uuid.end()) {
      return false;
    }

    return mapIt->second.first == T::ID;
  }

  /**
   * @brief Unload a resource by UUID
   * @tparam T Resource type
   * @param uuid Resource UUID
   * @return true if unloaded
   */
  template <typename T> bool unload(const std::string &uuid) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");

    auto typeIt = m_resources.find(T::ID);
    if (typeIt == m_resources.end()) {
      return false;
    }

    auto &typeResources = typeIt->second;
    auto resourceIt = typeResources.find(uuid);
    if (resourceIt == typeResources.end()) {
      return false;
    }

    // ✅ Free the RID
    auto rid = getRidByUuid<T>(uuid);
    if (rid.isValid()) {
      m_rid_allocator.free(rid);
      m_rid_to_uuid.erase(rid.id);
    }

    resourceIt->second->unload();
    typeResources.erase(resourceIt);
    return true;
  }

  /**
   * @brief Release a resource (decrement refcount, unload if zero)
   * @tparam T Resource type
   * @param resourceId Resource UUID
   */
  template <typename T> void release(const std::string &resourceId) {
    auto it = refCounts.find(T::ID);
    if (it != refCounts.end()) {
      auto data = it->second.find(resourceId);
      if (data != it->second.end()) {
        data->second.refCount--;

        if (data->second.refCount <= 0) {
          // Free RID
          auto rid = getRidByUuid<T>(resourceId);
          if (rid.isValid()) {
            m_rid_allocator.free(rid);
            m_rid_to_uuid.erase(rid.id);
          }

          // Unload and erase
          for (auto &[type, typeResources] : m_resources) {
            auto resourceIt = typeResources.find(resourceId);
            if (resourceIt != typeResources.end()) {
              resourceIt->second->unload();
              typeResources.erase(resourceIt);
              break;
            }
          }
          refCounts.erase(it);
        }
      }
    }
  }

  /**
   * @brief Get the RID for a resource by UUID
   * @tparam T Resource type
   * @param uuid Resource UUID
   * @return RID or RID::INVALID if not found
   */
  template <typename T> RID getRid(const std::string &uuid) const {
    for (const auto &[rid_id, mapData] : m_rid_to_uuid) {
      if (mapData.first == T::ID && mapData.second == uuid) {
        return RID{rid_id};
      }
    }
    return RID::INVALID;
  }

  /**
   * @brief Get the UUID for a resource by RID
   * @tparam T Resource type
   * @param rid Resource ID
   * @return UUID or empty string if not found
   */
  template <typename T> std::string getUuid(RID rid) const {
    auto it = m_rid_to_uuid.find(rid.id);
    if (it == m_rid_to_uuid.end()) {
      return "";
    }

    if (it->second.first != T::ID) {
      return ""; // Wrong type
    }

    return it->second.second;
  }

  /**
   * @brief Get the RID allocator (for advanced use)
   */
  RIDAllocator &getRidAllocator() { return m_rid_allocator; }

  /**
   * @brief Unload all resources
   */
  void clear();

  /**
   * @brief Get number of live resources
   */
  size_t getResourceCount() const {
    size_t count = 0;
    for (const auto &[type_id, typeResources] : m_resources) {
      count += typeResources.size();
    }
    return count;
  }

private:
  // Helper: get RID by UUID (internal)
  template <typename T> RID getRidByUuid(const std::string &uuid) const {
    for (const auto &[rid_id, mapData] : m_rid_to_uuid) {
      if (mapData.first == T::ID && mapData.second == uuid) {
        return RID{rid_id};
      }
    }
    return RID::INVALID;
  }

  // === Resource Storage ===

  /// High-level resources by type and UUID
  std::unordered_map<ResourceId,
                     std::unordered_map<std::string, std::shared_ptr<Resource>>>
      m_resources;

  /// Reference counting for automatic lifecycle management
  std::unordered_map<ResourceId, std::unordered_map<std::string, ResourceData>>
      refCounts;

  // === RID Management ===

  /// ✅ Central RID allocator (one RID per resource for all backends)
  RIDAllocator m_rid_allocator;

  /// RID → (ResourceType, UUID) mapping for GPU-level access
  std::unordered_map<uint64_t, std::pair<ResourceId, std::string>>
      m_rid_to_uuid;

  // === Devices ===

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
