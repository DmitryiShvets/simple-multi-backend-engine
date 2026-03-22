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

namespace ssme {

class Resource;
class RenderDevice;

struct ResourceSlot {
  std::unique_ptr<Resource> ptr = nullptr; // Владение памятью
  uint32_t generation = 0;                 // Счетчик "жизней" этого слота
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

    // 2. Если нет — создаем новый слот (Exclusive lock)
    std::unique_lock lock(m_map_mutex);
    index = m_slots.size();
    m_slots.emplace_back();
    m_uuid_to_idx[uuid] = index;

    auto &slot = m_slots[index];
    // Allocate unified RID for all backends
    std::vector<RID> rids = m_rid_allocator.allocate(T::COMPONENTS);

    std::lock_guard s_lock(get_stripe(index));

    slot.ptr =
        std::make_unique<T>(uuid, m_device_refs, std::forward<Args>(args)...);
    auto &resource = slot.ptr;
    slot.generation++; // Увеличиваем поколение при создании
    gen = slot.generation;

    resource->incrementUsersCount();
    // Setup RIDs in the resource (if it has one)
    resource->setup(rids);

    if (!resource->load()) {
      m_rid_allocator.free(rids); // Free RIDs on failure
      throw std::runtime_error("Failed to load resource: " + uuid);
    }

    return ResourceHandle<T>(uuid, index, gen, this);
  }

  bool unload(uint32_t index, uint32_t expected_gen) {
    std::lock_guard s_lock(get_stripe(index));
    return _unload(index, expected_gen);
  }

  bool _unload(uint32_t index, uint32_t expected_gen) {
    // Безопасность: проверяем валидность индкса
    auto slot = get_slot(index);
    debug_assert(slot != nullptr,
                 "Invalide index of Resource or Corrupted Resource Manager!");
    // Безопасность: проверяем, живой ли ресурс и то ли это поколение
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
    // Check if resource already exists by UUID
    std::lock_guard s_lock(get_stripe(index));
    // Безопасность: проверяем валидность индкса
    auto slot = get_slot(index);
    debug_assert(slot != nullptr,
                 "Invalide index of Resource or Corrupted Resource Manager!");
    // Безопасность: проверяем, живой ли ресурс и то ли это поколение
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
    // Безопасность: проверяем, живой ли ресурс и то ли это поколение
    if (slot && slot->ptr && slot->generation == expected_gen) {
      auto users = slot->ptr->decrementUsersCount();
      if (users == 0) { // Был последний
        _unload(index, expected_gen);
      }
    }
  }

  // Вспомогательный метод для хендла
  Resource *access(uint32_t index, uint32_t gen) {
    // Мы не блокируем тут мьютекс для скорости get()
    // Но проверяем поколение. Если ресурс удалят в этот миг -
    // это Race Condition, который решается Deferred Deletion (ниже).
    // auto &slot = m_slots[index];
    auto slot = get_slot(index);
    if (!slot)
      return nullptr;

    return (slot->generation == gen) ? slot->ptr.get() : nullptr;
  }

  bool increment_ref(uint32_t index, uint32_t gen) {
    std::lock_guard lock(get_stripe(index));
    auto slot = get_slot(index);
    debug_assert(slot != nullptr,
                 "Invalide index of Resource or Corrupted Resource Manager!");
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

private:
  // === Resource Storage ===
  static constexpr size_t STRIPE_COUNT = 64;
  std::mutex m_stripes[STRIPE_COUNT]; // Массив "полосок" блокировки

  // Таблица всех ресурсов в системе
  std::vector<ResourceSlot> m_slots;
  // Быстрый поиск индекса по имени (только при загрузке)
  std::unordered_map<std::string, uint32_t> m_uuid_to_idx;
  // Для защиты самой мапы
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
