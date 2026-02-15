#pragma once
#include "i_gpu_context_strategy.h"
#include "i_main_window.h"

class ImGuiContext;
class GLFWwindow;
class GLFWmonitor;

namespace Window {

class GLFWMainWindow : public IMainWindow {

public:
  explicit GLFWMainWindow(const WindowConfig &cfg)
      : config(cfg), m_window(nullptr) {}

  void init(const IGpuContextStrategy &contextStrategy) override;
  void destroy() override;
  bool shouldClose() const override;
  void swapBuffers() override;
  void update() override;
  void setPosition(int x, int y) override;

  void setMouseCallback(MouseCallback callback) override;
  void setKeyCallback(KeyCallback callback) override;
  void setCursorCallback(CursorCallback callback) override;
  void setResizeCallback(ResizeCallback callback) override;
  void setScrollCallback(ScrollCallback callback) override;
  void setCharCallback(CharCallback callback) override;
  void setWindowFocusCallback(WindowFocusCallback callback) override;
  void setCursorEnterCallback(CursorEnterCallback callback) override;

  void *getNativeWindow() const override;
  void setUiContext(void *ctx) override;

private:
  GLFWwindow *m_window = nullptr;
  ImGuiContext *m_ui_context = nullptr;
  WindowConfig config;

  MouseCallback m_mouseCallback = nullptr;
  KeyCallback m_keyCallback = nullptr;
  CursorCallback m_cursorCallback = nullptr;
  ResizeCallback m_resizeCallback = nullptr;
  ScrollCallback m_scrollCallback = nullptr;
  CharCallback m_charCallback = nullptr;
  WindowFocusCallback m_windowFocusCallback = nullptr;
  CursorEnterCallback m_cursorEnterCallback = nullptr;

  static int s_active_windows; // Counter for active windows

  static int toGLFWKey(Key key);
  static Key fromGLFWKey(int glfwKey);

  static void mouseButtonClickCallback(GLFWwindow *window, int button,
                                       int action, int mods);
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);
  static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
  static void charCallback(GLFWwindow *window, unsigned int c);
  static void windowFocusCallback(GLFWwindow *window, int focused);
  static void cursorEnterCallback(GLFWwindow *window, int entered);
  static void monitorCallback(GLFWmonitor *monitor, int event);

  static void errorHandlerCallback(int error, const char *description);

  static void resizeCallback(GLFWwindow *window, int width, int height);

  // Унаследовано через IMainWindow
  WindowConfig getConfig() override;
};
} // namespace Window
