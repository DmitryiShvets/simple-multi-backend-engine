#pragma once

#include "resource.h"
#include "resource_handle.h"

#include "core/gpu_types.h"
#include "core/rid_allocator.h"
#include "utils/debug_assert.h"

#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/resource_types.h"
#include "resource_loader.h"

#include <nlohmann/json.hpp>

namespace ssme {

class Resource;
class RenderDevice;

struct ResourceSlot {
  std::unique_ptr<Resource> ptr = nullptr;   // real memory
  uint32_t generation = 0;                  // garbage wall
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

  template <typename T>
  ResourceHandle<T> load(const std::string &path) {
      static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

      // Check cache first
      {
          std::shared_lock lock(m_map_mutex);
          if (auto it = m_uuid_to_idx.find(path); it != m_uuid_to_idx.end()) {
              uint32_t index = it->second;
              std::lock_guard s_lock(get_stripe(index));
              auto &slot = m_slots[index];
              if (slot.ptr && slot.generation > 0) {
                  slot.ptr->incrementUsersCount();
                  return ResourceHandle<T>(path, index, slot.generation, this);
              }
          }
      }

      // Find loader for this resource type
      auto it_loader = m_loaders.find(T::ID);
      if (it_loader == m_loaders.end()) {
          throw std::runtime_error("No loader registered for resource type ID: " + std::to_string((int)T::ID));
      }

      // Create params structure (T must have inner ParamsType, e.g. MaterialParams)
      typename T::ParamsType params;

      // Loader parses JSON and fills params with dependencies
      if (!it_loader->second->load(path, *this, &params)) {
          throw std::runtime_error("Loader failed to process file: " + path);
      }

      // Call creation logic with filled params
      return load<T, const typename T::ParamsType&>(path, params);
  }

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
    uint32_t index;
    uint32_t gen;
    // Check if resource already exists by UUID
    {
      std::shared_lock lock(m_map_mutex);
      if (auto it = m_uuid_to_idx.find(uuid); it != m_uuid_to_idx.end()) {
        index = it->second;
        std::lock_guard s_lock(get_stripe(index));
        auto &slot = m_slots[index];
        gen = slot.generation;
        if (slot.ptr) {
          slot.ptr->incrementUsersCount();
          return ResourceHandle<T>(uuid, index, gen, this);
        }
      }
    }

    // Create new slot with exclusive lock
    std::unique_lock lock(m_map_mutex);
    index = static_cast<uint32_t>(m_slots.size());
    m_slots.emplace_back();
    m_uuid_to_idx[uuid] = index;

    auto &slot = m_slots[index];

    std::lock_guard s_lock(get_stripe(index));

    slot.ptr =
        std::make_unique<T>(uuid, m_device_refs, std::forward<Args>(args)...);
    auto &resource = slot.ptr;
    slot.generation++;
    gen = slot.generation;

    uint32_t comp_required = resource->prepare();

    // Allocate unified RID for all backends
    std::vector<RID> rids = m_rid_allocator.allocate(comp_required);

    // Setup RIDs in the resource (if it has one)
    resource->setup(rids);

    if (!resource->load()) {
      m_rid_allocator.free(rids); // Free RIDs on failure
      throw std::runtime_error("Failed to load resource: " + uuid);
    }

    resource->incrementUsersCount();

    return ResourceHandle<T>(uuid, index, gen, this);
  }

  bool unload(uint32_t index, uint32_t expected_gen) {
    std::lock_guard s_lock(get_stripe(index));
    return _unload(index, expected_gen);
  }

  bool _unload(uint32_t index, uint32_t expected_gen) {
    auto slot = get_slot(index);
    debug_assert(slot != nullptr,
                 "Invalid index or corrupted Resource Manager!");
    if (!slot || !slot->ptr || slot->generation != expected_gen)
      return false;
    auto rids = slot->ptr->components();
    slot->ptr->unload();        // GPU Cleanup
    slot->ptr.reset();          // CPU Cleanup
    slot->generation++;         // Invalidate all handlers
    m_rid_allocator.free(rids); // Free all RIDs

    return true;
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
    uint32_t index;
    // Check if resource already exists by UUID
    std::shared_lock lock(m_map_mutex);
    auto it = m_uuid_to_idx.find(uuid);
    // OK case because this version of getter is public
    if (it == m_uuid_to_idx.end())
      return nullptr;
    index = it->second;
    std::lock_guard s_lock(get_stripe(index));
    auto &slot = m_slots[index];
    if (!slot.ptr)
      return nullptr;
    return static_cast<T *>(slot.ptr.get());
  }

  template <typename T> T *get(uint32_t index, uint32_t expected_gen) {
    std::lock_guard s_lock(get_stripe(index));
    auto slot = get_slot(index);
    debug_assert(slot != nullptr,
                 "Invalid index or corrupted Resource Manager!");
    if (!slot || !slot->ptr || slot->generation != expected_gen)
      return nullptr;
    return static_cast<T *>(slot->ptr.get());
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
    uint32_t index;
    return get<T>(uuid) != nullptr;
  }

  template <typename T> bool has(uint32_t index, uint32_t expected_gen) {
    static_assert(std::is_base_of<Resource, T>::value,
                  "T must derive from Resource");
    return get<T>(index, expected_gen) != nullptr;
  }

  void release(uint32_t index, uint32_t expected_gen) {
    std::mutex &m = get_stripe(index);
    std::lock_guard s_lock(m);
    auto slot = get_slot(index);
    // Safety: check if resource is alive and has expected generation
    if (slot && slot->ptr && slot->generation == expected_gen) {
      auto users = slot->ptr->decrementUsersCount();
      if (users == 0) { // Was the last reference
        _unload(index, expected_gen);
      }
    }
  }

  // Helper method for handle
  Resource *access(uint32_t index, uint32_t gen) {
    // We don't lock mutex here for speed of get()
    // But check generation. If resource is deleted at this moment -
    // this is a Race Condition, solved by Deferred Deletion (below).
    auto slot = get_slot(index);
    if (!slot)
      return nullptr;

    return (slot->generation == gen) ? slot->ptr.get() : nullptr;
  }

  bool increment_ref(uint32_t index, uint32_t gen) {
    std::lock_guard lock(get_stripe(index));
    auto slot = get_slot(index);
    debug_assert(slot != nullptr,
                 "Invalid index or corrupted Resource Manager!");
    if (slot && slot->ptr && slot->generation == gen) {
      slot->ptr->incrementUsersCount();
      return true;
    }
    return false;
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
   * @brief Print for debug
   */
  void printStats() const;

  // Registration (done once at engine startup)
  void registerLoader(std::unique_ptr<IResourceLoader> loader);

private:
  // === Resource Storage ===
  static constexpr size_t STRIPE_COUNT = 64;
  std::mutex m_stripes[STRIPE_COUNT]; // Lock striping array
  // All available loaders
  std::unordered_map<ResourceId, std::unique_ptr<IResourceLoader>> m_loaders;

  // Table of all resources in the system
  std::vector<ResourceSlot> m_slots;
  // Fast lookup by name (only during loading)
  std::unordered_map<std::string, uint32_t> m_uuid_to_idx;
  // For protecting the map itself
  std::shared_mutex m_map_mutex;

  std::mutex &get_stripe(uint32_t index) {
    return m_stripes[index % STRIPE_COUNT];
  }

  ResourceSlot *get_slot(uint32_t index) {
    if (index >= m_slots.size())
      return nullptr;
    return &m_slots[index];
  }
  // === RID Management ===

  /// ✅ Central RID allocator (one RID per resource for all backends)
  RIDAllocator m_rid_allocator{RIDRange::USER_START, RIDRange::USER_END};

  // === Devices ===

  std::vector<RenderDevice *> m_devices;
  VecRefRD m_device_refs;
};
} // namespace ssme
