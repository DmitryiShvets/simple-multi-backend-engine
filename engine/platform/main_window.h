#pragma once

#include "core/gpu_types.h"
#include "gpu_context_strategy.h"
#include <functional>
#include <string>

namespace ssme {

/**
 * @brief Keyboard key codes.
 */
enum class Key {
    Escape,
    Enter,
    Space,
    Num1,
    Num2,
    Num3,
    Num4,
    // Add more as needed
};

/**
 * @brief Mouse button codes.
 */
enum class MouseButton { Left, Right, Middle };

/**
 * @brief Button/key actions.
 */
enum class Action { Press, Release, Repeat };


/**
 * @brief Window configuration for platform creation.
 */
struct WindowConfig {
    std::string title;
    int width;
    int height;
};

/**
 * @brief Abstract interface for main window.
 */
class MainWindow {
public:
    using MouseCallback = std::function<void(MouseButton, Action, int, double, double)>;
    using KeyCallback = std::function<void(Key, Action, int)>;
    using CursorCallback = std::function<void(int, int)>;
    using ResizeCallback = std::function<void(int, int)>;
    using ScrollCallback = std::function<void(double, double)>;
    using CharCallback = std::function<void(unsigned int)>;
    using WindowFocusCallback = std::function<void(bool)>;
    using CursorEnterCallback = std::function<void(bool)>;

    virtual ~MainWindow() = default;

    /**
     * @brief Initialize window with GPU context strategy.
     */
    virtual void init(const GpuContextStrategy& contextStrategy) = 0;

    /**
     * @brief Clean up window resources.
     */
    virtual void cleanup() = 0;
    /**
     * @brief Close window.
     */
    virtual void destroy() = 0;

    /**
     * @brief Check if window should close.
     */
    virtual bool shouldClose() const = 0;

    /**
     * @brief Swap front and back buffers.
     */
    virtual void swapBuffers() = 0;

    /**
     * @brief Process window events.
     */
    virtual void update() = 0;

    /**
     * @brief Set window position on screen.
     */
    virtual void setPosition(int x, int y) = 0;

    // ==================== Callbacks ====================

    virtual void setMouseCallback(MouseCallback callback) = 0;
    virtual void setKeyCallback(KeyCallback callback) = 0;
    virtual void setCursorCallback(CursorCallback callback) = 0;
    virtual void setResizeCallback(ResizeCallback callback) = 0;
    virtual void setScrollCallback(ScrollCallback callback) = 0;
    virtual void setCharCallback(CharCallback callback) = 0;
    virtual void setWindowFocusCallback(WindowFocusCallback callback) = 0;
    virtual void setCursorEnterCallback(CursorEnterCallback callback) = 0;

    // ==================== Accessors ====================

    virtual WindowConfig getConfig() = 0;
    virtual void* getNativeWindow() const = 0;
    virtual void* getNativeHwnd() const = 0;
    virtual void setUiContext(void* ctx) = 0;
    virtual GpuBackend getGpuBackend() = 0;
};

} // namespace ssme
