#pragma once

#include "core/gpu_types.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ssme {
class MainWindow;
/**
 * @brief Interface for platform-specific functionality with multi-window
 * support.
 */
class Platform {
public:
  Platform() = default;
  virtual ~Platform() = default;

  // ==================== Window Management ====================

  /**
   * @brief Initialize platform and create first window.
   * @param appName Application name (also used as first window title).
   * @param width Window width.
   * @param height Window height.
   * @return True if initialization was successful.
   */
  virtual bool initialize(const std::string &appName, int width,
                          int height) = 0;

  /**
   * @brief Add a new window to the platform.
   * @param title Window title.
   * @param width Window width.
   * @param height Window height.
   */
  virtual void addWindow(const std::string &title, int width, int height, GpuBackend type) = 0;

  /**
   * @brief Remove a window by index.
   * @param index Window index.
   */
  virtual void removeWindow(size_t index) = 0;

  /**
   * @brief Get number of windows.
   * @return Number of windows.
   */
  virtual size_t getWindowCount() const = 0;

  /**
   * @brief Check if all windows are alive.
   * @return True if all windows are open.
   */
  virtual bool allWindowsAlive() const = 0;

  /**
   * @brief Update opengl buffers to show frame (like swap chain present.).
   */
  virtual void swapOpenGLBuffers() = 0;

  /**
   * @brief Update all windows (process events, etc.).
   */
  virtual void updateAllWindows() = 0;

  /**
   * @brief Clean up all platform resources.
   */
  virtual void cleanup() = 0;

  /**
   * @brief Destroy all windows.
   */
  virtual void destroy() = 0;

  // ==================== Per-Window Access ====================

  /**
   * @brief Get window size for specific window.
   * @param index Window index.
   * @param width Pointer to store width.
   * @param height Pointer to store height.
   */
  virtual void getWindowSize(size_t index, int *width, int *height) const = 0;
  // Get window by backend type
  virtual MainWindow &getWindow(GpuBackend type) = 0;
  virtual const MainWindow &getWindow(GpuBackend type) const = 0;

  /**
   * @brief Check if specific window has been resized.
   * @param index Window index.
   * @return True if window was resized.
   */
  virtual bool hasWindowResized(size_t index) const = 0;

  /**
   * @brief Create Vulkan surface for specific window.
   * @param index Window index.
   * @param instance Vulkan instance (void* for abstraction).
   * @return Surface handle (void* for abstraction).
   */
  virtual void *createVulkanSurface(void *instance) = 0;

  /**
   * @brief Get required Vulkan instance extensions.
   * @return Vector of extension names.
   */
  virtual std::vector<const char *>
  getRequiredVulkanInstanceExtensions() const = 0;

  // ==================== Callbacks (global for all windows)
  // ====================

  /**
   * @brief Set callback for window resize events.
   * @param callback Function called when any window is resized.
   *                 Parameters: (windowIndex, width, height)
   */
  virtual void setResizeCallback(
      std::function<void(size_t windowIndex, int width, int height)>
          callback) = 0;

  /**
   * @brief Set callback for mouse input events.
   * @param callback Function called on mouse input.
   *                 Parameters: (windowIndex, x, y, button)
   */
  virtual void setMouseCallback(
      std::function<void(size_t windowIndex, float x, float y, uint32_t button)>
          callback) = 0;

  /**
   * @brief Set callback for keyboard input events.
   * @param callback Function called on keyboard input.
   *                 Parameters: (windowIndex, key, pressed)
   */
  virtual void setKeyboardCallback(
      std::function<void(size_t windowIndex, uint32_t key, bool pressed)>
          callback) = 0;

  /**
   * @brief Set callback for character input events.
   * @param callback Function called on character input.
   *                 Parameters: (windowIndex, codepoint)
   */
  virtual void setCharCallback(
      std::function<void(size_t windowIndex, uint32_t codepoint)> callback) = 0;

  // ==================== Window Title ====================

  /**
   * @brief Set window title.
   * @param index Window index.
   * @param title New title.
   */
  virtual void setWindowTitle(size_t index, const std::string &title) = 0;

  virtual void setWindowPosition(GpuBackend type, std::pair<int, int> position) = 0;

};

} // namespace ssme
