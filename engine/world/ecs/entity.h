#pragma once
#include "core/time.h"
#include <string>
#include <type_traits>

namespace ssme {

class Component;

/**
 * @brief Entity class that can have multiple components attached to it.
 *
 * Entities are containers for components. They don't have any behavior
 * on their own, but gain functionality through the components attached to them.
 */
class Entity {
private:
  std::string name;
  bool active = true;

public:
  /**
   * @brief Constructor with a name.
   * @param name The name of the entity.
   */
  explicit Entity(const std::string &name) : name(name) {}

  /**
   * @brief Virtual destructor for proper cleanup.
   */
  virtual ~Entity() = default;

  /**
   * @brief Get the name of the entity.
   * @return The name of the entity.
   */
  const std::string &getName() const { return name; }

  /**
   * @brief Check if the entity is active.
   * @return True if the entity is active, false otherwise.
   */
  bool isActive() const { return active; }

  /**
   * @brief Set the active state of the entity.
   * @param isActive The new active state.
   */
  void setActive(bool isActive) { active = isActive; }

  /**
   * @brief Update all components of the entity.
   * @param deltaTime The time elapsed since the last frame.
   */
  void update(TimeDelta deltaTime);

  /**
   * @brief Add a component to the entity.
   * @tparam T The type of component to add.
   * @tparam Args The types of arguments to pass to the component constructor.
   * @param args The arguments to pass to the component constructor.
   * @return A pointer to the newly created component.
   */
  template <typename T, typename... Args> T *addComponent(Args &&...args) {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");
    // NOT IMPLEMENTED
  }

  /**
   * @brief Get a component of a specific type.
   * @tparam T The type of component to get.
   * @return A pointer to the component, or nullptr if not found.
   */
  template <typename T> T *getComponent() const {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");
    // NOT IMPLEMENTED
  }

  /**
   * @brief Remove a component of a specific type.
   * @tparam T The type of component to remove.
   * @return True if the component was removed, false otherwise.
   */
  template <typename T> bool removeComponent() {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    // NOT IMPLEMENTED
  }

  /**
   * @brief Check if the entity has a component of a specific type.
   * @tparam T The type of component to check for.
   * @return True if the entity has the component, false otherwise.
   */
  template <typename T> bool HasComponent() const {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");
    return GetComponent<T>() != nullptr;
  }
};

} // namespace ssme
