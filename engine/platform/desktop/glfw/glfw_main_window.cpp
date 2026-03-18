#include "main_window.h"
#include "glfw_main_window.h"
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include "imgui_internal.h"
#include "utils/logger.h"

#include <imgui_internal.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace ssme {
    // Constructor implementation
    GLFWMainWindow::GLFWMainWindow(const WindowConfig& cfg)
        : m_config(cfg), m_window(nullptr), m_ui_context(nullptr) {
    }

    // Destructor implementation
    GLFWMainWindow::~GLFWMainWindow() {
        destroy();
    }
// Initialize the static counter
int GLFWMainWindow::s_active_windows = 0;

void GLFWMainWindow::init(const GpuContextStrategy &contextStrategy) {
// Force GLFW to use X11 backend (XWayland) to allow window positioning on
// linux.
#if defined(__linux__)
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif
  // Only initialize GLFW if it's the first window
  if (s_active_windows == 0) {
    if (!glfwInit()) {
      Logger::error_log("Failed to initialize GLFW!");
      exit(EXIT_FAILURE);
    }
  }

  // Use the provided strategy to prepare window hints
  contextStrategy.prepareWindowCreationHints();
  m_backend_type = contextStrategy.getGpuBackend();
  m_window = glfwCreateWindow(m_config.width, m_config.height, m_config.title.c_str(),
                              NULL, NULL);
  if (!m_window) {
    Logger::error_log("Failed to create window!");
    if (s_active_windows == 0) {
      glfwTerminate();
    }
    exit(EXIT_FAILURE);
  }

  // Increment the counter now that the window is created
  s_active_windows++;

  // Use the provided strategy to create the context
  if (!contextStrategy.createContext(this->getNativeWindow())) {
    Logger::error_log("Failed to create GPU context!");
    destroy();
    exit(EXIT_FAILURE);
  }

  glfwSetWindowUserPointer(m_window, this);
  glfwSetErrorCallback(errorHandlerCallback);
  glfwSetFramebufferSizeCallback(m_window, resizeCallback);
  glfwSetKeyCallback(m_window, keyCallback);
  glfwSetMouseButtonCallback(m_window, mouseButtonClickCallback);
  glfwSetCursorPosCallback(m_window, cursorPosCallback);
  glfwSetScrollCallback(m_window, scrollCallback);
  glfwSetCharCallback(m_window, charCallback);
  glfwSetWindowFocusCallback(m_window, windowFocusCallback);
  glfwSetCursorEnterCallback(m_window, cursorEnterCallback);
  glfwSetMonitorCallback(monitorCallback);
}

void GLFWMainWindow::destroy() {
  if (m_window) {
    glfwDestroyWindow(m_window);
    m_window = nullptr;
    s_active_windows--;
  }

  if (s_active_windows == 0) {
    glfwTerminate();
  }
}

void GLFWMainWindow::swapBuffers() {
  if (m_window)
    glfwSwapBuffers(m_window);
}

void GLFWMainWindow::update() {
  glfwPollEvents();
}

bool GLFWMainWindow::shouldClose() const {
  return glfwWindowShouldClose(m_window);
}

void GLFWMainWindow::setPosition(int x, int y) {
  glfwSetWindowPos(m_window, x, y);
}

void GLFWMainWindow::setCursorCallback(CursorCallback cb) {
  m_cursorCallback = cb;
}

void GLFWMainWindow::setResizeCallback(ResizeCallback cb) {
  m_resizeCallback = cb;
}

void GLFWMainWindow::setKeyCallback(KeyCallback cb) {
  m_keyCallback = cb;
}

void GLFWMainWindow::setMouseCallback(MouseCallback cb) {
  m_mouseCallback = cb;
}

void GLFWMainWindow::setScrollCallback(ScrollCallback cb) {
  m_scrollCallback = cb;
}

void GLFWMainWindow::setCharCallback(CharCallback cb) {
  m_charCallback = cb;
}

void GLFWMainWindow::setWindowFocusCallback(WindowFocusCallback cb) {
  m_windowFocusCallback = cb;
}

void GLFWMainWindow::setCursorEnterCallback(CursorEnterCallback cb) {
  m_cursorEnterCallback = cb;
}

void GLFWMainWindow::keyCallback(GLFWwindow *window, int key, int scancode,
                                 int action, int mods) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  bool event_was_captured = false;

  // Handle ImGui callbacks
  if (self && self->m_ui_context) {
    ImGui::SetCurrentContext(self->m_ui_context);
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    event_was_captured = ImGui::GetIO().WantCaptureKeyboard;
  }

  if (self && self->m_keyCallback && !event_was_captured) {
    Action act = (action == GLFW_PRESS)     ? Action::Press
                 : (action == GLFW_RELEASE) ? Action::Release
                                            : Action::Repeat;

    self->m_keyCallback(fromGLFWKey(key), act, mods);
  }
}

void GLFWMainWindow::mouseButtonClickCallback(GLFWwindow *window, int button,
                                              int action, int mods) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  bool event_was_captured = false;

  // Handle ImGui callbacks
  if (self && self->m_ui_context) {
    ImGui::SetCurrentContext(self->m_ui_context);
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    event_was_captured = ImGui::GetIO().WantCaptureMouse;
  }

  // Handle user callbacks
  if (self && self->m_mouseCallback && !event_was_captured) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    MouseButton mb = (button == GLFW_MOUSE_BUTTON_LEFT) ? MouseButton::Left
                                                        : MouseButton::Right;
    Action act = (action == GLFW_PRESS) ? Action::Press : Action::Release;

    self->m_mouseCallback(mb, act, mods, xpos, ypos);
  }
}

void GLFWMainWindow::cursorPosCallback(GLFWwindow *window, double xpos,
                                       double ypos) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  bool event_was_captured = false;

  // Handle ImGui callbacks
  if (self && self->m_ui_context) {
    ImGui::SetCurrentContext(self->m_ui_context);
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    event_was_captured = ImGui::GetIO().WantCaptureMouse;
  }

  if (self && self->m_cursorCallback && !event_was_captured) {
    self->m_cursorCallback(xpos, ypos);
  }
}

void GLFWMainWindow::scrollCallback(GLFWwindow *window, double xoffset,
                                    double yoffset) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  bool event_was_captured = false;

  if (self && self->m_ui_context) {
    ImGui::SetCurrentContext(self->m_ui_context);
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    event_was_captured = ImGui::GetIO().WantCaptureMouse;
  }

  if (self && self->m_scrollCallback && !event_was_captured) {
    self->m_scrollCallback(xoffset, yoffset);
  }
}

void GLFWMainWindow::charCallback(GLFWwindow *window, unsigned int c) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  bool event_was_captured = false;

  if (self && self->m_ui_context) {
    ImGui::SetCurrentContext(self->m_ui_context);
    ImGui_ImplGlfw_CharCallback(window, c);
    event_was_captured = ImGui::GetIO().WantCaptureKeyboard;
  }

  if (self && self->m_charCallback && !event_was_captured) {
    self->m_charCallback(c);
  }
}

void GLFWMainWindow::windowFocusCallback(GLFWwindow *window, int focused) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));

  if (self && self->m_ui_context) {
    ImGui::SetCurrentContext(self->m_ui_context);
    ImGui_ImplGlfw_WindowFocusCallback(window, focused);
  }

  if (self && self->m_windowFocusCallback) {
    self->m_windowFocusCallback(focused);
  }
}

void GLFWMainWindow::cursorEnterCallback(GLFWwindow *window, int entered) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));

  if (self && self->m_ui_context) {
    ImGui::SetCurrentContext(self->m_ui_context);
    ImGui_ImplGlfw_CursorEnterCallback(window, entered);
  }

  if (self && self->m_cursorEnterCallback) {
    self->m_cursorEnterCallback(entered);
  }
}

void GLFWMainWindow::monitorCallback(GLFWmonitor *monitor, int event) {
  // Global callback - call ImGui implementation directly
  ImGui_ImplGlfw_MonitorCallback(monitor, event);
}

void GLFWMainWindow::errorHandlerCallback(int error, const char *description) {
  Logger::error_log(description);
}

void GLFWMainWindow::resizeCallback(GLFWwindow *window, int width, int height) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  self->m_config.width = width;
  self->m_config.height = height;

  if (self->m_resizeCallback) {
    self->m_resizeCallback(width, height);
  }
}

void *GLFWMainWindow::getNativeWindow() const {
  return m_window;
}

void GLFWMainWindow::setUiContext(void *ctx) {
  m_ui_context = static_cast<ImGuiContext *>(ctx);
}

WindowConfig GLFWMainWindow::getConfig() {
  return m_config;
}

GpuBackend GLFWMainWindow::getGpuBackend() {
    return m_backend_type;
}

Key GLFWMainWindow::fromGLFWKey(int glfwKey) {
  switch (glfwKey) {
  case GLFW_KEY_ESCAPE:
    return Key::Escape;
  case GLFW_KEY_ENTER:
    return Key::Enter;
  case GLFW_KEY_1:
    return Key::Num1;
  case GLFW_KEY_2:
    return Key::Num2;
  default:
    return Key::Escape;
  }
}

int GLFWMainWindow::toGLFWKey(Key key) {
  switch (key) {
  case Key::Escape:
    return GLFW_KEY_ESCAPE;
  case Key::Enter:
    return GLFW_KEY_ENTER;
  case Key::Num1:
    return GLFW_KEY_1;
  case Key::Num2:
    return GLFW_KEY_2;
  default:
    return GLFW_KEY_UNKNOWN;
  }
}

} // namespace ssme
