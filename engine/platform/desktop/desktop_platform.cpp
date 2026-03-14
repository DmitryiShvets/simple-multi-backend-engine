#include "desktop_platform.h"
#include "desktop/backends/glfw/glfw_main_window.h"
#include "desktop/backends/glfw/glfw_gpu_context_creator.h"

namespace ssme {

DesktopPlatform::~DesktopPlatform() {
    cleanup();
}

// ==================== Window Management ====================

bool DesktopPlatform::initialize(const std::string& appName, int width, int height) {
    auto window = createWindow(appName, width, height);
    if (!window) {
        return false;
    }

    m_windows.push_back(std::move(window));
    m_windowResized.push_back(false);
    m_windowWidths.push_back(width);
    m_windowHeights.push_back(height);

    return true;
}

void DesktopPlatform::addWindow(const std::string& title, int width, int height) {
    auto window = createWindow(title, width, height);
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

size_t DesktopPlatform::getWindowCount() const {
    return m_windows.size();
}

bool DesktopPlatform::allAlive() const {
    if (m_windows.empty()) {
        return false;
    }

    for (const auto& window : m_windows) {
        if (window->shouldClose()) {
            return false;
        }
    }
    return true;
}

void DesktopPlatform::updateAllWindows() {
    for (auto& window : m_windows) {
        window->update();
    }
}

void DesktopPlatform::cleanup() {
    for (auto& window : m_windows) {
        window->destroy();
    }
    m_windows.clear();
    m_windowResized.clear();
    m_windowWidths.clear();
    m_windowHeights.clear();
}

// ==================== Per-Window Access ====================

void DesktopPlatform::getWindowSize(size_t index, int* width, int* height) const {
    if (index >= m_windows.size()) {
        return;
    }

    if (width) *width = m_windowWidths[index];
    if (height) *height = m_windowHeights[index];
}

bool DesktopPlatform::hasWindowResized(size_t index) const {
    if (index >= m_windows.size()) {
        return false;
    }
    return m_windowResized[index];
}

void* DesktopPlatform::createVulkanSurface(size_t index, void* instance) {
    if (index >= m_windows.size()) {
        return nullptr;
    }

    // TODO: Implement Vulkan surface creation using the window's native handle
    // This requires access to Vulkan instance and proper surface creation
    return nullptr;
}

std::vector<const char*> DesktopPlatform::getRequiredVulkanInstanceExtensions() const {
    // TODO: Get actual extensions from GLFW
    return {};
}

// ==================== Callbacks ====================

void DesktopPlatform::setResizeCallback(
    std::function<void(size_t, int, int)> callback
) {
    m_resizeCallback = std::move(callback);
}

void DesktopPlatform::setMouseCallback(
    std::function<void(size_t, float, float, uint32_t)> callback
) {
    m_mouseCallback = std::move(callback);
}

void DesktopPlatform::setKeyboardCallback(
    std::function<void(size_t, uint32_t, bool)> callback
) {
    m_keyboardCallback = std::move(callback);
}

void DesktopPlatform::setCharCallback(
    std::function<void(size_t, uint32_t)> callback
) {
    m_charCallback = std::move(callback);
}

// ==================== Window Title ====================

void DesktopPlatform::setWindowTitle(size_t index, const std::string& title) {
    if (index >= m_windows.size()) {
        return;
    }

    // TODO: Set window title via IMainWindow interface
    // For now, this would need to be added to IMainWindow
}

// ==================== Internal Helpers ====================

std::unique_ptr<MainWindow> DesktopPlatform::createWindow(
    const std::string& title, int width, int height
) {
    WindowConfig config{title, width, height};
    auto window = std::make_unique<GLFWMainWindow>(config);

    // Initialize with OpenGL context strategy
    // TODO: Make this configurable (OpenGL/Vulkan)
    OpenGLGpuContextCreator context_creator;
    window->init(context_creator);

    // Set up callbacks
    size_t window_index = m_windows.size();

    window->setResizeCallback(
        [this, window_index](int w, int h) {
            onWindowResize(window_index, w, h);
        }
    );

    window->setMouseCallback(
        [this, window_index](MouseButton button, Action action,
                            int mods, double x, double y) {
            onWindowMouse(window_index, button, action, x, y);
        }
    );

    window->setKeyCallback(
        [this, window_index](Key key, Action action, int scancode) {
            onWindowKey(window_index, key, action, scancode);
        }
    );

    window->setCharCallback(
        [this, window_index](unsigned int codepoint) {
            onWindowChar(window_index, codepoint);
        }
    );

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
        m_mouseCallback(index, static_cast<float>(x), static_cast<float>(y), button_code);
    }
}

void DesktopPlatform::onWindowKey(size_t index, Key key,
                                   Action action, int /*scancode*/) {
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

} // namespace ssme
