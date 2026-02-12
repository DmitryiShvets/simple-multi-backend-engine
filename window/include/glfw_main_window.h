#pragma once
#include "i_gpu_context_strategy.h"
#include "i_main_window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Window {

class GLFWMainWindow : public IMainWindow {

public:
  explicit GLFWMainWindow(const WindowConfig &cfg)
      : config(cfg), m_window(nullptr) {}

  void initialize(const IGpuContextStrategy &contextStrategy) override;
  void destroy() override;
  bool shouldClose() const override;
  void swapBuffers() override;
  void pollEvents() override;
  void setPosition(int x, int y) override;

  void setMouseCallback(MouseCallback callback) override;
  void setKeyCallback(KeyCallback callback) override;
  void setCursorCallback(CursorCallback callback) override;
  void setResizeCallback(ResizeCallback callback) override;

  void *getNativeWindow() const override;

private:
  GLFWwindow *m_window = nullptr;
  WindowConfig config;

  MouseCallback m_mouseCallback = nullptr;
  KeyCallback m_keyCallback = nullptr;
  CursorCallback m_cursorCallback = nullptr;
  ResizeCallback m_resizeCallback = nullptr;

  static int s_active_windows; // Counter for active windows

  static int toGLFWKey(Key key);
  static Key fromGLFWKey(int glfwKey);

  static void mouseButtonClickCallback(GLFWwindow *window, int button,
                                       int action, int mods);
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);

  static void errorHandlerCallback(int error, const char *description);

  static void resizeCallback(GLFWwindow *window, int width, int height);

  // Унаследовано через IMainWindow
  WindowConfig getConfig() override;
};
} // namespace Window
