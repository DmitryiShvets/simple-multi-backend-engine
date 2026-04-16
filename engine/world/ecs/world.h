#pragma once
#include "entity.h"
#include "world_base.h"

#include <functional>
#include <vector>

namespace ssme {
class System; // Forward declaration
class Scene;

class World : public WorldBase {
  friend class System;
  friend class Scene;

public:
  // 0. Entities
  Entity createEntity(const std::string &name = "") {
    EntityID id = WorldBase::createEntity(name);
    return Entity(id, *this);
  }

  Entity getEntity(const std::string &name) {
    EntityID id = WorldBase::getEntity(name);
    return Entity(id, *this);
  }

  bool isEntityAlive(Entity entity) const {
    return WorldBase::isEntityAlive(entity.id());
  }

  bool isEntityAlive(EntityID id) const { return WorldBase::isEntityAlive(id); }

  bool destroyEntity(Entity entity) {
    return WorldBase::destroyEntity(entity.id());
  }
  bool destroyEntity(EntityID id) { return WorldBase::destroyEntity(id); }
  bool destroyEntity(const std::string &name) {
    auto e = getEntity(name);
    return WorldBase::destroyEntity(e.id());
  }

  // 1. Work with struct tags (compile-time)
  template <typename T> void addTag(Entity entity) {
    // Use existing mechanism,
    // but pass nullptr and size=0 for efficiency
    using PureT = std::remove_cvref_t<T>;
    _addComponentRaw(entity.id(), typeid(PureT).hash_code(),
                     typeid(PureT).name(), 0, 1, nullptr, nullptr, false);
  }

  template <typename T> bool hasTag(Entity entity) const {
    return hasComponent<T>(entity);
  }

  template <typename T> void removeTag(Entity entity) {
    removeComponent<T>(entity);
  }
  // 2. Work with dynamic tags (runtime by name)
  void addTag(Entity entity, const std::string &tag_name) {
    // Find or create tag entity
    EntityID tag_id = WorldBase::getEntity(tag_name);
    if (tag_id == 0) {
      tag_id = WorldBase::createEntity(tag_name);
    }
    _addTagRaw(entity.id(), tag_id);
  }

  bool hasTag(Entity entity, const std::string &tag_name) const {
    EntityID tag_id = WorldBase::getEntity(tag_name);
    if (tag_id == 0)
      return false;
    return _hasTagRaw(entity.id(), tag_id);
  }

  bool hasTag(EntityID entity, const std::string &tag_name) const {
    EntityID tag_id = WorldBase::getEntity(tag_name);
    if (tag_id == 0)
      return false;
    return _hasTagRaw(entity, tag_id);
  }

  void removeTag(Entity entity, const std::string &tag_name) {
    EntityID tag_id = WorldBase::getEntity(tag_name);
    if (tag_id != 0) {
      _removeTagRaw(entity.id(), tag_id);
    }
  }
  // 3. Work by direct ID (fastest path)
  void addTag(Entity entity, EntityID tag_id) {
    _addTagRaw(entity.id(), tag_id);
  }
  void addTag(EntityID entity, EntityID tag_id) { _addTagRaw(entity, tag_id); }

  bool hasTag(Entity entity, EntityID tag_id) const {
    return _hasTagRaw(entity.id(), tag_id);
  }

  bool hasTag(EntityID entity, EntityID tag_id) const {
    return _hasTagRaw(entity, tag_id);
  }

  void removeTag(Entity entity, EntityID tag_id) {
    _removeTagRaw(entity.id(), tag_id);
  }

  void removeTag(EntityID entity, EntityID tag_id) {
    _removeTagRaw(entity, tag_id);
  }

  // 4. Components (Type Erasure)
  template <typename T, typename... Args>
  bool add(Entity entity, Args &&...args) {
    T data(std::forward<Args>(args)...);
    return add(entity, std::move(data));
  }

  template <typename T>
  bool addComponent(Entity entity,
                    T &&data) { // T&& here is universal reference
    // 1. Extract pure type (without & and const)
    using PureT = std::remove_cvref_t<T>;

    // 2. Check if input object is temporary (rvalue)
    // If T is not a reference, then it's rvalue (std::move was used)
    bool is_move = !std::is_lvalue_reference_v<T>;

    // 3. Call low-level method
    return _addComponentRaw(entity.id(), typeid(PureT).hash_code(),
                            typeid(PureT).name(), sizeof(PureT), alignof(PureT),
                            (const void *)&data, getLifecycle<PureT>(),
                            is_move);
  }

  template <typename T> T *getComponent(EntityID id) {
    using PureT = std::remove_cvref_t<T>;
    return static_cast<T *>(_getComponentRaw(id, typeid(PureT).hash_code()));
  }

  template <typename T> T *getComponent(Entity entity) {
    using PureT = std::remove_cvref_t<T>;
    return static_cast<T *>(
        _getComponentRaw(entity.id(), typeid(PureT).hash_code()));
  }

  template <typename T> bool hasComponent(EntityID id) const {
    return _hasComponentRaw(id, typeid(T).hash_code());
  }
  template <typename T> bool hasComponent(Entity entity) const {
    return _hasComponentRaw(entity.id(), typeid(T).hash_code());
  }
  template <typename T> bool removeComponent(Entity entity) {
    return _removeComponentRaw(entity.id(), typeid(T).hash_code());
  }

  using ObserveCallback = std::function<void(EntityID, World &)>;
  template <typename T> void observeAdd(ObserveCallback cb) {
    _internalObserveAdd(
        typeid(T).hash_code(), typeid(T).name(), sizeof(T), alignof(T),
        [this, cb](EntityID id) { cb(id, *this); }, getLifecycle<T>());
  }
  template <typename T> void observeSet(ObserveCallback cb) {
    _internalObserveSet(
        typeid(T).hash_code(), typeid(T).name(), sizeof(T), alignof(T),
        [this, cb](EntityID id) { cb(id, *this); }, getLifecycle<T>());
  }

  template <typename T> void observeRemove(ObserveCallback cb) {
    _internalObserveRemove(typeid(T).hash_code(),
                           [this, cb](EntityID id) { cb(id, *this); });
  }

  void observeDestroy(ObserveCallback cb) {
    _internalObserveDestroy([this, cb](EntityID id) { cb(id, *this); });
  }

  template <typename... Comps> class View {
    World *m_world;
    std::vector<size_t> m_hashes;
    std::vector<EntityID> m_tag_ids;

  public:
    View(World *w) : m_world(w) {
      (m_hashes.push_back(typeid(Comps).hash_code()), ...);
    }

    // Add filter by struct tag
    template <typename T> View &withTag() {
      m_hashes.push_back(typeid(std::remove_cvref_t<T>).hash_code());
      return *this;
    }

    // Add filter by dynamic tag (by name)
    View &withTag(const std::string &tag_name) {
      EntityID tag_id = m_world->getEntity(tag_name).id();
      if (tag_id != 0) {
        m_tag_ids.push_back(tag_id);
      }
      return *this;
    }

    // Add filter by direct ID
    View &withTag(EntityID tag_id) {
      m_tag_ids.push_back(tag_id);
      return *this;
    }

    void each(std::function<void(EntityID, Comps &...)> func) {
      m_world->_viewImplRaw(m_hashes, m_tag_ids, [&](EntityID id, void **ptrs) {
        _invoke(func, id, ptrs, std::index_sequence_for<Comps...>{});
      });
    }

    // ✅ New version with checking
    void each_safe(std::function<void(EntityID, Comps &...)> func) {
      m_world->_viewImplRawSafe(
          m_hashes, m_tag_ids,
          [&](EntityID id, Comps &...comps) { func(id, comps...); });
    }

  private:
    template <size_t... Is>
    void _invoke(std::function<void(EntityID, Comps &...)> &f, EntityID id,
                 void **p, std::index_sequence<Is...>) {
      f(id, *static_cast<Comps *>(p[Is])...);
    }
  };

  template <typename... Comps> auto view() { return View<Comps...>(this); }
};

} // namespace ssme
