#pragma once

#include "resource_handle_base.h"

namespace ssme {

/**
 * @brief Template class for resource handles.
 * @tparam T The type of resource.
 */
template <typename T> class ResourceHandle : public ResourceHandleBase {
public:
  /**
   * @brief Default constructor.
   */
  ResourceHandle() = default;
  /**
   * @brief Constructor with a resource UUID and resource manager.
   * @param uuid The resource UUID.
   * @param manager The resource manager.
   */
  ResourceHandle(const std::string &uuid, uint32_t index, uint32_t gen,
                 ResourceManager *mgr)
      : ResourceHandleBase(uuid, index, gen, mgr) {}

  ResourceHandle(const ResourceHandle& other) = default;
  ResourceHandle& operator=(const ResourceHandle& other) = default;
  ResourceHandle(ResourceHandle&& other) noexcept = default;
  ResourceHandle& operator=(ResourceHandle&& other) noexcept = default;

  ~ResourceHandle() = default;
  /**
   * @brief Get the resource.
   * @return A pointer to the resource, or nullptr if not found.
   */
  T *get() const { return static_cast<T *>(fetch()); }

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

private:
  T *m_cached_ptr = nullptr;
};

} // namespace ssme
