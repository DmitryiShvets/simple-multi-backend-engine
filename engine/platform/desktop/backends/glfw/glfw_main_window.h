#pragma once

#include "main_window.h"
#include "gpu_context_strategy.h"

class ImGuiContext;
class GLFWwindow;
class GLFWmonitor;

namespace ssme {

/**
 * @brief GLFW-based main window implementation.
 */
class GLFWMainWindow : public MainWindow {
public:
    explicit GLFWMainWindow(const WindowConfig& cfg);
    ~GLFWMainWindow() override;

    // ==================== MainWindow Interface ====================

    void init(const GpuContextStrategy& contextStrategy) override;
    void destroy() override;
    bool shouldClose() const override;
    void swapBuffers() override;
    void update() override;
    void setPosition(int x, int y) override;

    // Callbacks
    void setMouseCallback(MouseCallback callback) override;
    void setKeyCallback(KeyCallback callback) override;
    void setCursorCallback(CursorCallback callback) override;
    void setResizeCallback(ResizeCallback callback) override;
    void setScrollCallback(ScrollCallback callback) override;
    void setCharCallback(CharCallback callback) override;
    void setWindowFocusCallback(WindowFocusCallback callback) override;
    void setCursorEnterCallback(CursorEnterCallback callback) override;

    // Accessors
    void* getNativeWindow() const override;
    void setUiContext(void* ctx) override;
    WindowConfig getConfig() override;

private:
    GLFWwindow* m_window = nullptr;
    ImGuiContext* m_ui_context = nullptr;
    WindowConfig m_config;

    // Stored callbacks
    MouseCallback m_mouseCallback;
    KeyCallback m_keyCallback;
    CursorCallback m_cursorCallback;
    ResizeCallback m_resizeCallback;
    ScrollCallback m_scrollCallback;
    CharCallback m_charCallback;
    WindowFocusCallback m_windowFocusCallback;
    CursorEnterCallback m_cursorEnterCallback;

    static int s_active_windows;  // Counter for active windows

    // GLFW to ssme conversion
    static int toGLFWKey(Key key);
    static Key fromGLFWKey(int glfwKey);

    // Static GLFW callbacks
    static void mouseButtonClickCallback(GLFWwindow* window, int button, int action, int mods);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void charCallback(GLFWwindow* window, unsigned int c);
    static void windowFocusCallback(GLFWwindow* window, int focused);
    static void cursorEnterCallback(GLFWwindow* window, int entered);
    static void monitorCallback(GLFWmonitor* monitor, int event);
    static void errorHandlerCallback(int error, const char* description);
    static void resizeCallback(GLFWwindow* window, int width, int height);
};

} // namespace ssme
