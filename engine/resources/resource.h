#pragma once

#include <functional>
#include <string>
#include <vector>

namespace ssme {

class RenderDevice;
using VecRefRD = std::vector<std::reference_wrapper<RenderDevice>>;

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

protected:
  virtual bool doLoad() = 0;
  virtual bool doUnload() = 0;
  // Unique identifier for this resource within the system
  std::string m_id;
  // User readebale identifier for this resource within the system
  std::string m_name;
  // Loading state flag for resource lifecycle management
  bool m_loaded = false;
  // Array of rhi device interface for creating/destoying resource
  VecRefRD m_devices;
};
} // namespace ssme
