#pragma once

#include <string>
namespace ssme {
/**
 * @brief Template class for resource handles.
 * @tparam T The type of resource.
 */
template <typename T> class ResourceHandle {
private:
  std::string m_uuid;
  class ResourceManager *m_resource_manager = nullptr;

public:
  /**
   * @brief Default constructor.
   */
  ResourceHandle() = default;

  ~ResourceHandle();
  /**
   * @brief Constructor with a resource UUID and resource manager.
   * @param uuid The resource UUID.
   * @param manager The resource manager.
   */
  ResourceHandle(const std::string &uuid, ResourceManager *manager)
      : m_uuid(uuid), m_resource_manager(manager) {}

  /**
   * @brief Get the resource.
   * @return A pointer to the resource, or nullptr if not found.
   */
  T *get() const;
  /**
   * @brief Check if the handle is valid.
   * @return True if the handle is valid, false otherwise.
   */
  bool isValid() const;
  /**
   * @brief Get the resource UUID.
   * @return The resource UUID.
   */
  const std::string &id() const { return m_uuid; }

  /**
   * @brief Convenience operator for accessing the resource.
   * @return A pointer to the resource.
   */
  T *operator->() const { return get(); }
  /**
   * @brief Convenience operator for dereferencing the resource.
   * @return A reference to the resource.
   */
  T &operator*() const { return *get(); }

  /**
   * @brief Convenience operator for checking if the handle is valid.
   * @return True if the handle is valid, false otherwise.
   */
  operator bool() const { return isValid(); }
};

} // namespace ssme
