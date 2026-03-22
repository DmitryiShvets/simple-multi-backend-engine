#pragma once

#include "core/atomic_numeric.h"
#include "core/rid.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ssme {

class RenderDevice;
using VecRefRD = std::vector<std::reference_wrapper<RenderDevice>>;
using VecRID = std::vector<RID>;

// Resource base class
class Resource {
private:
public:
  /**
   * @brief Constructor with a resource ID.
   * @param id The unique identifier for the resource.
   */
  explicit Resource(const std::string &id, const VecRefRD &devices)
      : m_id(id), m_devices(devices) {}
  /**
   * @brief Virtual destructor for proper cleanup.
   */
  virtual ~Resource() = default;
  /**
   * @brief Get the resource ID.
   * @return The resource ID.
   */
  const std::string &id() const { return m_id; }
  /**
   * @brief Get the resource user-readeble label.
   * @return The resource label.
   */
  const std::string &name() const { return m_name; }
  /**
   * @brief Get array of gpu resources IDs.
   * @return The vector of RIDs.
   */
  const VecRID &components() const { return m_rids; }
  /**
   * @brief Check if the resource is loaded.
   * @return True if the resource is loaded, false otherwise.
   */
  bool isLoaded() const { return m_loaded; }
  /**
   * @brief Load the resource.
   * @return True if the resource was m_loaded successfully, false otherwise.
   */
  bool load() {
    m_loaded = doLoad();
    return m_loaded;
  }
  /**
   * @brief Unload the resource.
   */
  void unload() {
    doUnload();
    m_loaded = false;
  }
  /**
   * @brief Set up gpu identifiers for the resources.
   */
  void setup(const VecRID &rids) { doSetup(rids); }

  uint64_t incrementUsersCount() { return m_users.increment(); }
  uint64_t decrementUsersCount() { return m_users.decrement(); }
  uint64_t getUsersCount() const { return m_users.get(); }

protected:
  virtual void doSetup(const VecRID &rids) = 0;
  virtual bool doLoad() = 0;
  virtual bool doUnload() = 0;

  AtomicNumeric<uint64_t> m_users;
  // Unique identifier for this resource within the system
  std::string m_id;
  // User readebale identifier for this resource within the system
  std::string m_name;
  // Loading state flag for resource lifecycle management
  bool m_loaded = false;
  // Array of rhi device interface for creating/destoying resource
  VecRefRD m_devices;
  VecRID m_rids;
};
} // namespace ssme
