#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace ssme {
using EntityID = uint64_t;

struct ecs_type_info_t;

struct TypeLifecycle {
  void (*ctor)(void *ptr, int32_t count,
               const ecs_type_info_t *type_info); // Default Constructor
  void (*dtor)(void *ptr, int32_t count,
               const ecs_type_info_t *type_info); // Destructor
  void (*copy)(void *dst, const void *src, int32_t count,
               const ecs_type_info_t *type_info); // Copy assignment
  void (*move)(void *dst, void *src, int32_t count,
               const ecs_type_info_t *type_info); // Move assignment
  void (*copy_ctor)(void *dst, const void *src, int32_t count,
                    const ecs_type_info_t *type_info); // Copy constructor
  void (*move_ctor)(void *dst, void *src, int32_t count,
                    const ecs_type_info_t *type_info); // Move constructor
  void (*emplace_copy)(void *dst, const void *src);
  void (*emplace_move)(void *dst, void *src);
};

template <typename T> static const TypeLifecycle *getLifecycle() {
  // 1. Optimization for simple types (int, float, POD structs)
  // If type can be copied via memcpy, Flecs will do it faster itself.
  if constexpr (std::is_trivially_copyable_v<T> &&
                std::is_trivially_destructible_v<T>) {
    return nullptr;
  }

  // 2. Static structure for non-trivial types (std::vector,
  // std::string, etc.) It will live in memory until the end of program.
  static const TypeLifecycle lc = {
      // ctor: Default Constructor (on raw memory)
      [](void *p, int32_t c, const ecs_type_info_t *) {
        T *ptr = static_cast<T *>(p);
        for (int32_t i = 0; i < c; ++i) {
          new (&ptr[i]) T();
        }
      },
      // dtor: Destructor
      [](void *p, int32_t c, const ecs_type_info_t *) {
        T *ptr = static_cast<T *>(p);
        for (int32_t i = 0; i < c; ++i) {
          ptr[i].~T();
        }
      },
      // copy: Copy Assignment (into LIVE object)
      [](void *d, const void *s, int32_t c, const ecs_type_info_t *) {
        T *dst = static_cast<T *>(d);
        const T *src = static_cast<const T *>(s);
        for (int32_t i = 0; i < c; ++i) {
          dst[i] = src[i];
        }
      },
      // move: Move Assignment (into LIVE object)
      [](void *d, void *s, int32_t c, const ecs_type_info_t *) {
        T *dst = static_cast<T *>(d);
        T *src = static_cast<T *>(s);
        for (int32_t i = 0; i < c; ++i) {
          dst[i] = std::move(src[i]);
        }
      },
      // copy_ctor: Copy Constructor (called when duplicating entities)
      [](void *d, const void *s, int32_t c, const ecs_type_info_t *) {
        T *dst = static_cast<T *>(d);
        const T *src = static_cast<const T *>(s);
        for (int32_t i = 0; i < c; ++i) {
          new (&dst[i]) T(src[i]);
        }
      },
      // move_ctor: Move Constructor (called when changing archetype/table)
      [](void *d, void *s, int32_t c, const ecs_type_info_t *) {
        T *dst = static_cast<T *>(d);
        T *src = static_cast<T *>(s);
        for (int32_t i = 0; i < c; ++i) {
          new (&dst[i]) T(std::move(src[i]));
        }
      },
      // emplace_copy:
      [](void *d, const void *s) { new (d) T(*static_cast<const T *>(s)); },
      // emplace_move:
      [](void *d, void *s) { new (d) T(std::move(*static_cast<T *>(s))); }};

  return &lc;
}

class WorldBase {
public:
  WorldBase();
  ~WorldBase();

  EntityID createEntity(const std::string &name = "");
  EntityID getEntity(const std::string &name = "") const;
  std::string getEntityName(EntityID id) const;
  bool destroyEntity(EntityID entity);
  bool destruct(EntityID id);
  bool isEntityAlive(EntityID entity) const;

  bool _addComponentRaw(EntityID id, size_t type_hash, const char *type_name,
                        size_t size, size_t align, const void *data,
                        const TypeLifecycle *lc, bool is_move);

  void *_getComponentRaw(EntityID e, size_t type_hash);
  bool _hasComponentRaw(EntityID e, size_t type_hash) const;
  bool _removeComponentRaw(EntityID e, size_t type_hash);

  bool _addTagRaw(EntityID id, EntityID tag_id);
  bool _hasTagRaw(EntityID id, EntityID tag_id) const;
  bool _removeTagRaw(EntityID id, EntityID tag_id);

  using RawEachFunc = std::function<void(EntityID, void **)>;
  void _viewImplRaw(const std::vector<size_t> &type_hashes,
                    const std::vector<EntityID> &tag_ids, RawEachFunc callback);

  template <typename... Comps>
  using SafeEachFunc = std::function<void(EntityID, Comps &...)>;
  template <typename... Comps>
  void _viewImplRawSafe(const std::vector<size_t> &type_hashes,
                        const std::vector<EntityID> &tag_ids,
                        SafeEachFunc<Comps...> callback);

  using ObserveCallback = std::function<void(EntityID)>;
  void _internalObserveAdd(size_t hash, const char *name, size_t s, size_t a,
                           ObserveCallback cb, const TypeLifecycle *lc);
  void _internalObserveSet(size_t hash, const char *name, size_t s, size_t a,
                           ObserveCallback cb, const TypeLifecycle *lc);
  void _internalObserveRemove(size_t hash, ObserveCallback cb);
  void _internalObserveDestroy(ObserveCallback cb);

private:
  struct Impl;
  Impl *m_impl;
};
} // namespace ssme
