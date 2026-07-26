#include "desktop_platform.h"
#include "core/gpu_types.h"
#include "desktop/glfw/glfw_gpu_context_creator.h"
#include "desktop/glfw/glfw_main_window.h"
#include <algorithm>
#include <stdexcept>

// This file requires the full GLFW implementation
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace ssme {

DesktopPlatform::~DesktopPlatform() { cleanup(); }

// ==================== Window Management ====================

bool DesktopPlatform::initialize(const std::string &appName, int width,
                                 int height) {
  return true;
}

void DesktopPlatform::addWindow(const std::string &title, int width, int height,
                                GpuBackend type) {
  auto window = createWindow(title, width, height, type);
  if (!window) {
    return;
  }

  m_windows.push_back(std::move(window));
  m_windowResized.push_back(false);
  m_windowWidths.push_back(width);
  m_windowHeights.push_back(height);
}

void DesktopPlatform::removeWindow(size_t index) {
  if (index >= m_windows.size()) {
    return;
  }

  m_windows.erase(m_windows.begin() + static_cast<long>(index));
  m_windowResized.erase(m_windowResized.begin() + static_cast<long>(index));
  m_windowWidths.erase(m_windowWidths.begin() + static_cast<long>(index));
  m_windowHeights.erase(m_windowHeights.begin() + static_cast<long>(index));
}

size_t DesktopPlatform::getWindowCount() const { return m_windows.size(); }

bool DesktopPlatform::allWindowsAlive() const {
  if (m_windows.empty()) {
    return false;
  }

  for (const auto &window : m_windows) {
    if (window->shouldClose()) {
      return false;
    }
  }
  return true;
}

void DesktopPlatform::updateAllWindows() {
  for (auto &window : m_windows) {
    window->update();
  }
}

void DesktopPlatform::destroy() {
  for (auto &window : m_windows) {
    window->destroy();
  }
}

void DesktopPlatform::cleanup() {
  m_windows.clear();
  m_windowResized.clear();
  m_windowWidths.clear();
  m_windowHeights.clear();
}

// ==================== Per-Window Access ====================

MainWindow &DesktopPlatform::getWindow(GpuBackend type) {
    auto it = std::find_if(m_windows.begin(), m_windows.end(),
        [type](auto &w) { return w->getGpuBackend() == type; });
    if (it == m_windows.end())
        throw std::runtime_error("Window not found");
    return **it;
}
MainWindow const &DesktopPlatform::getWindow(GpuBackend type) const {
    auto it = std::find_if(m_windows.begin(), m_windows.end(),
        [type](auto &w) { return w->getGpuBackend() == type; });
    if (it == m_windows.end())
        throw std::runtime_error("Window not found");
    return **it;
}

void DesktopPlatform::getWindowSize(size_t index, int *width,
                                    int *height) const {
  if (index >= m_windows.size()) {
    return;
  }

  if (width)
    *width = m_windowWidths[index];
  if (height)
    *height = m_windowHeights[index];
}

bool DesktopPlatform::hasWindowResized(size_t index) const {
  if (index >= m_windows.size()) {
    return false;
  }
  return m_windowResized[index];
}

void *DesktopPlatform::createVulkanSurface(void *instance) {
  auto vk_window =
      std::find_if(m_windows.cbegin(), m_windows.cend(), [](const auto &e) {
        return e->getGpuBackend() == GpuBackend::Vulkan;
      });
  if (vk_window == m_windows.end()) {
    // Found Vulkan window
    throw std::runtime_error("Failed to create window surface! Failed to find "
                             "suitable Vulkan-context window.");
  }
  MainWindow *window = vk_window->get();
  VkInstance vkInstance = static_cast<VkInstance>(instance);
  VkSurfaceKHR surface;

  glfwCreateWindowSurface(vkInstance, (GLFWwindow *)window->getNativeWindow(),
                          nullptr, &surface);
  return surface;
}

std::vector<const char *>
DesktopPlatform::getRequiredVulkanInstanceExtensions() const {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  return extensions;
}

// ==================== Callbacks ====================

void DesktopPlatform::setResizeCallback(
    std::function<void(size_t, int, int)> callback) {
  m_resizeCallback = std::move(callback);
}

void DesktopPlatform::setMouseCallback(
    std::function<void(size_t, float, float, uint32_t)> callback) {
  m_mouseCallback = std::move(callback);
}

void DesktopPlatform::setKeyboardCallback(
    std::function<void(size_t, uint32_t, bool)> callback) {
  m_keyboardCallback = std::move(callback);
}

void DesktopPlatform::setCharCallback(
    std::function<void(size_t, uint32_t)> callback) {
  m_charCallback = std::move(callback);
}

// ==================== Window Title ====================

void DesktopPlatform::setWindowTitle(size_t index, const std::string &title) {
  if (index >= m_windows.size()) {
    return;
  }

  // TODO: Set window title via IMainWindow interface
  // For now, this would need to be added to IMainWindow
}

// ==================== Internal Helpers ====================

std::unique_ptr<MainWindow>
DesktopPlatform::createWindow(const std::string &title, int width, int height,
                              GpuBackend type) {
  WindowConfig config{title, width, height};
  auto window = std::make_unique<GLFWMainWindow>(config);

  // Initialize with OpenGL context strategy
  // TODO: Make this configurable (OpenGL/Vulkan)
  OpenGLGpuContextCreator gl_gpu_ctx_creator;
  VulkanGpuContextCreator vk_gpu_ctx_creator;
  Dx12GpuContextCreator dx_gpu_ctx_creator;
  if (type == GpuBackend::OpenGL) {
    window->init(gl_gpu_ctx_creator);
  }
  else if (type == GpuBackend::DirectX12) {
    window->init(dx_gpu_ctx_creator);
  }
  else {
    window->init(vk_gpu_ctx_creator);
  }

  // Set up callbacks
  size_t window_index = m_windows.size();

  window->setResizeCallback([this, window_index](int w, int h) {
    onWindowResize(window_index, w, h);
  });

  window->setMouseCallback([this, window_index](MouseButton button,
                                                Action action, int mods,
                                                double x, double y) {
    onWindowMouse(window_index, button, action, x, y);
  });

  window->setKeyCallback(
      [this, window_index](Key key, Action action, int scancode) {
        onWindowKey(window_index, key, action, scancode);
      });

  window->setCharCallback([this, window_index](unsigned int codepoint) {
    onWindowChar(window_index, codepoint);
  });

  return window;
}

void DesktopPlatform::onWindowResize(size_t index, int width, int height) {
  if (index < m_windows.size()) {
    m_windowResized[index] = true;
    m_windowWidths[index] = width;
    m_windowHeights[index] = height;

    if (m_resizeCallback) {
      m_resizeCallback(index, width, height);
    }
  }
}

void DesktopPlatform::onWindowMouse(size_t index, MouseButton button,
                                    Action action, double x, double y) {
  if (m_mouseCallback) {
    uint32_t button_code = static_cast<uint32_t>(button);
    m_mouseCallback(index, static_cast<float>(x), static_cast<float>(y),
                    button_code);
  }
}

void DesktopPlatform::onWindowKey(size_t index, Key key, Action action,
                                  int /*scancode*/) {
  if (m_keyboardCallback) {
    uint32_t key_code = static_cast<uint32_t>(key);
    bool pressed = (action == Action::Press);
    m_keyboardCallback(index, key_code, pressed);
  }
}

void DesktopPlatform::onWindowChar(size_t index, unsigned int codepoint) {
  if (m_charCallback) {
    m_charCallback(index, codepoint);
  }
}

void DesktopPlatform::setWindowPosition(GpuBackend type, std::pair<int, int> position) {
    auto& window = getWindow(type);
    window.setPosition(position.first, position.second);
}

void DesktopPlatform::swapOpenGLBuffers() {
  auto gl_window =
      std::find_if(m_windows.cbegin(), m_windows.cend(), [](const auto &e) {
        return e->getGpuBackend() == GpuBackend::OpenGL;
      });
  if (gl_window == m_windows.end()) {
        // NOTE: USER CAN DISABLE GL WINODW. IT'S OK
        return;
  }
  gl_window->get()->swapBuffers();
}

} // namespace ssme
