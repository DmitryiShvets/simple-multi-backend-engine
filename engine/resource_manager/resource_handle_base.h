#pragma once

#include <cstdint>
#include <string>

namespace ssme {

// Forward declaration
class ResourceManager;
class Resource;
constexpr uint32_t INVALID_SLOT = 0xFFFFFFFF;
constexpr uint32_t INVALID_GEN = 0;
/**
 * @brief A non-template handle base
 * Contains all the logic for communicating with the manager
 */
class ResourceHandleBase {
public:
  /**
   * @brief Default constructor.
   */
  ResourceHandleBase() = default;
  /**
   * @brief Constructor with a resource UUID and resource manager.
   * @param uuid The resource UUID.
   * @param manager The resource manager.
   */
  ResourceHandleBase(const std::string &uuid, uint32_t index, uint32_t gen,
                     ResourceManager *mgr);

  ~ResourceHandleBase();
  /**
   * @brief Copy constructor.
   */
  ResourceHandleBase(const ResourceHandleBase &other);
  /**
   * @brief Copy assignment operator.
   */
  ResourceHandleBase &operator=(const ResourceHandleBase &other);
  /**
   * @brief Move constructor.
   */
  ResourceHandleBase(ResourceHandleBase &&other) noexcept;
  /**
   * @brief Move assignment operator.
   */
  ResourceHandleBase &operator=(ResourceHandleBase &&other) noexcept;
  /**
   * @brief Check if the handle is valid.
   * @return True if the handle is valid, false otherwise.
   */
  bool isValid() const;
  /**
   * @brief Get the resource UUID.
   * @return The resource UUID.
   */
  const std::string &uuid() const { return m_uuid; }

protected:
  Resource *fetch() const;

  std::string m_uuid;
  uint32_t m_slot_index = INVALID_SLOT;
  uint32_t m_expected_gen = INVALID_GEN;
  ResourceManager *m_resource_manager = nullptr;
};

} // namespace ssme
