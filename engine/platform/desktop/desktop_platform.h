#pragma once

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
    void addWindow(const std::string& title, int width, int height) override;
    void removeWindow(size_t index) override;
    size_t getWindowCount() const override;
    bool allAlive() const override;
    void updateAllWindows() override;
    void cleanup() override;

    // ==================== Per-Window Access ====================

    void getWindowSize(size_t index, int* width, int* height) const override;
    bool hasWindowResized(size_t index) const override;
    void* createVulkanSurface(size_t index, void* instance) override;
    std::vector<const char*> getRequiredVulkanInstanceExtensions() const override;

    // ==================== Callbacks ====================

    void setResizeCallback(
        std::function<void(size_t, int, int)> callback
    ) override;

    void setMouseCallback(
        std::function<void(size_t, float, float, uint32_t)> callback
    ) override;

    void setKeyboardCallback(
        std::function<void(size_t, uint32_t, bool)> callback
    ) override;

    void setCharCallback(
        std::function<void(size_t, uint32_t)> callback
    ) override;

    // ==================== Window Title ====================

    void setWindowTitle(size_t index, const std::string& title) override;

private:
    // All windows
    std::vector<std::unique_ptr<MainWindow>> m_windows;

    // Resize tracking per window
    std::vector<bool> m_windowResized;
    std::vector<int> m_windowWidths;
    std::vector<int> m_windowHeights;

    // Callbacks
    std::function<void(size_t, int, int)> m_resizeCallback;
    std::function<void(size_t, float, float, uint32_t)> m_mouseCallback;
    std::function<void(size_t, uint32_t, bool)> m_keyboardCallback;
    std::function<void(size_t, uint32_t)> m_charCallback;

    // Internal helper to create window
    std::unique_ptr<MainWindow> createWindow(
        const std::string& title, int width, int height);

    // Internal callback adapters
    void onWindowResize(size_t index, int width, int height);
    void onWindowMouse(size_t index, MouseButton button,
                       Action action, double x, double y);
    void onWindowKey(size_t index, Key key, Action action, int scancode);
    void onWindowChar(size_t index, unsigned int codepoint);
};

} // namespace ssme
