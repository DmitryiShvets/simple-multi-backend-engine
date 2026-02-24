#pragma once
#include "concepts.h"
#include "ecs_types.h"

#include <string>
#include <unordered_map>
#include <utility>

namespace Core::Ecs {
// Apply the concept directly to the class for better strictness
template <typename WorldImpl>
  requires EcsWorld<WorldImpl>
class World {
public:
  // Constructor that forwards arguments to the implementation
  template <typename... Args>
  World(Args &&...args) : m_impl(std::forward<Args>(args)...) {}

  EntityHandle createEntity(const std::string &name = {}) {
    EntityHandle entity = impl().createEntity();
    m_entyties[name] = entity;
    return entity;
  }

  EntityHandle getEntity(const std::string &name) { return m_entyties[name]; }

  template <typename Component, typename... Args>
  void addComponent(EntityHandle entity, Args &&...args) {
    impl().template addComponent<Component>(entity,
                                            std::forward<Args>(args)...);
  }

  template <typename Component> Component &getComponent(EntityHandle entity) {
    return impl().template getComponent<Component>(entity);
  }

  template <typename Component> bool hasComponent(EntityHandle entity) {
    return impl().template hasComponent<Component>(entity);
  }

  template <typename... Components> auto createQuery() {
    return impl().template createQuery<Components...>();
  }

  auto &getNativeWorld() { return impl().getNativeWorld(); }

private:
  WorldImpl &impl() { return m_impl; }
  const WorldImpl &impl() const { return m_impl; }

  WorldImpl m_impl;
  std::unordered_map<std::string, EntityHandle> m_entyties;
};
} // namespace Core::Ecs
