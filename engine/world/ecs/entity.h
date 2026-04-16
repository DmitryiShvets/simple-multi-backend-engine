#pragma once
#include "entity_base.h"
#include "world_base.h"
#include <string>
#include <typeinfo>
#include <utility>

namespace ssme {
/**
 * @brief Entity class that can have multiple components attached to it.
 *
 * Entities are containers for components. They don't have any behavior
 * on their own, but gain functionality through the components attached to them.
 */
class Entity : private EntityBase {

public:
  /**
   * @brief Constructor with a name.
   * @param name The name of the entity.
   */
  explicit Entity(EntityID id, WorldBase &world) : EntityBase(id, world) {}

  EntityID id() const { return m_id; }

  std::string name() const { return getName(); }
  /**
   * @brief Add a component to the entity.
   * @tparam T The type of component to add.
   * @tparam Args The types of arguments to pass to the component constructor.
   * @param args The arguments to pass to the component constructor.
   * @return A pointer to the newly created component.
   */
  template <typename T, typename... Args> bool add(Args &&...args) {
    T data(std::forward<Args>(args)...);
    return add(std::move(data));
  }

  template <typename T> bool add(T &&data) {
    // 1. Extract pure type (without & and const)
    using PureT = std::remove_cvref_t<T>;

    // 2. Check if input object is temporary (rvalue)
    // If T is not a reference, then it's rvalue (std::move was used)
    bool is_move = !std::is_lvalue_reference_v<T>;

    // 3. Call low-level method
    return addComponentRaw(typeid(PureT).hash_code(), typeid(PureT).name(),
                           sizeof(PureT), alignof(PureT), (const void *)&data,
                           getLifecycle<PureT>(), is_move);
  }
  /**
   * @brief Get a component of a specific type.
   * @tparam T The type of component to get.
   * @return A pointer to the component, or nullptr if not found.
   */
  template <typename T> T *get() const {
    using PureT = std::remove_cvref_t<T>;
    return static_cast<T *>(getComponentRaw(typeid(PureT).hash_code()));
  }

  /**
   * @brief Remove a component of a specific type.
   * @tparam T The type of component to remove.
   * @return True if the component was removed, false otherwise.
   */
  template <typename T> bool remove() {
    return removeComponentRaw(typeid(T).hash_code());
  }

  /**
   * @brief Check if the entity has a component of a specific type.
   * @tparam T The type of component to check for.
   * @return True if the entity has the component, false otherwise.
   */
  template <typename T> bool has() const {
    return hasComponentRaw(typeid(T).hash_code());
  }
};

} // namespace ssme
