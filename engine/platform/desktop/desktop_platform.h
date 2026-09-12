#pragma once

#include "core/gpu_types.h"
#include "platform.h"
#include "main_window.h"  // From old platform code
#include <memory>
#include <vector>
#include <functional>

namespace ssme {

/**
 * @brief Desktop implementation
 */
class DesktopPlatform final : public Platform {
public:
    DesktopPlatform() = default;
    ~DesktopPlatform() override;

    // ==================== Window Management ====================

    bool initialize(const std::string& appName, int width, int height) override;
    void addWindow(const std::string& title, int width, int height, GpuBackend type) override;
    void removeWindow(size_t index) override;
    size_t getWindowCount() const override;
    bool allWindowsAlive() const override;
    void updateAllWindows() override;
    void swapOpenGLBuffers() override;
    void cleanup() override;
    void destroy() override;

    // ==================== Per-Window Access ====================

    void getWindowSize(size_t index, int* width, int* height) const override;
    bool hasWindowResized(size_t index) const override;
    void* createVulkanSurface(void* instance) override;
    MainWindow &getWindow(GpuBackend type) override;
    const MainWindow &getWindow(GpuBackend type) const override;
    std::vector<const char*> getRequiredVulkanInstanceExtensions() const override;

    // ==================== Callbacks ====================

    void setResizeCallback(
        std::function<void(size_t, int, int)> callback
    ) override;

    void setCharCallback(
        std::function<void(size_t, uint32_t)> callback
    ) override;

    void setKeyCallback(
        std::function<void(size_t, Key, KeyActionType, int)> callback
    ) override;

    void setMouseButtonCallback(
        std::function<void(size_t, MouseButton, KeyActionType, int, double, double)> callback
    ) override;

    void setCursorPosCallback(
        std::function<void(size_t, double, double)> callback
    ) override;

    void setScrollCallback(
        std::function<void(size_t, double, double)> callback
    ) override;

    // ==================== Window Title ====================

    void setWindowTitle(size_t index, const std::string& title) override;

    void setWindowPosition(GpuBackend type, std::pair<int, int> position) override;

private:
    // All windows
    std::vector<std::unique_ptr<MainWindow>> m_windows;

    // Resize tracking per window
    std::vector<bool> m_windowResized;
    std::vector<int> m_windowWidths;
    std::vector<int> m_windowHeights;

    // Callbacks
    std::function<void(size_t, int, int)> m_resizeCallback;
    std::function<void(size_t, Key, KeyActionType, int)> m_keyCallback;
    std::function<void(size_t, MouseButton, KeyActionType, int, double, double)> m_mouseButtonCallback;
    std::function<void(size_t, double, double)> m_cursorPosCallback;
    std::function<void(size_t, double, double)> m_scrollCallback;
    std::function<void(size_t, uint32_t)> m_charCallback;

    // Internal helper to create window
    std::unique_ptr<MainWindow> createWindow(
        const std::string& title, int width, int height, GpuBackend type);

    // Internal callback adapters
    void onWindowResize(size_t index, int width, int height);
    void onWindowMouse(size_t index, MouseButton button, KeyActionType action,
                       int mods, double x, double y);
    void onWindowKey(size_t index, Key key, KeyActionType action, int mods);
    void onWindowCursor(size_t index, double x, double y);
    void onWindowScroll(size_t index, double xoffset, double yoffset);
    void onWindowChar(size_t index, unsigned int codepoint);
};

} // namespace ssme
