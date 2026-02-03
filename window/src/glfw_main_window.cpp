#include "glfw_main_window.h"
#include "i_main_window.h"
#include "logger.h"

namespace Window {

void GLFWMainWindow::initialize(const IGpuContextStrategy& contextStrategy) {
  if (!glfwInit()) {
    Logger::error_log("Не удалось иницализировать GLFW!");
    exit(EXIT_FAILURE);
  }

  // Use the provided strategy to prepare window hints
  contextStrategy.prepareWindowCreationHints();

  m_window = glfwCreateWindow(config.width, config.height, config.title.c_str(),
                              NULL, NULL);
  if (!m_window) {
    Logger::error_log("Не удалось создать окно!");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Use the provided strategy to create the context
  if (!contextStrategy.createContext(this->getNativeWindow())) {
    Logger::error_log("Не удалось создать GPU контекст!");
    glfwDestroyWindow(m_window);
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSwapInterval(1);

  glfwSetWindowUserPointer(m_window, this);
  glfwSetErrorCallback(errorHandlerCallback);
  glfwSetFramebufferSizeCallback(m_window, resizeCallback);
  glfwSetKeyCallback(m_window, keyCallback);
  glfwSetMouseButtonCallback(m_window, mouseButtonClickCallback);
  glfwSetCursorPosCallback(m_window, cursorPosCallback);
}

void GLFWMainWindow::destroy() {
  glfwSetWindowShouldClose(m_window, 1);
  glfwTerminate();
}
void GLFWMainWindow::swapBuffers() { glfwSwapBuffers(m_window); }
void GLFWMainWindow::pollEvents() { glfwPollEvents(); }

bool GLFWMainWindow::shouldClose() const {
  return glfwWindowShouldClose(m_window);
}

void GLFWMainWindow::setCursorCallback(CursorCallback cb) {
  m_cursorCallback = cb;
}

void GLFWMainWindow::setResizeCallback(ResizeCallback cb) {
  m_resizeCallback = cb;
}

void GLFWMainWindow::setKeyCallback(KeyCallback cb) { m_keyCallback = cb; }

void GLFWMainWindow::setMouseCallback(MouseCallback cb) {
  m_mouseCallback = cb;
}

void GLFWMainWindow::keyCallback(GLFWwindow *window, int key, int scancode,
                                 int action, int mods) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  if (self && self->m_keyCallback) {
    Action act = (action == GLFW_PRESS)     ? Action::Press
                 : (action == GLFW_RELEASE) ? Action::Release
                                            : Action::Repeat;

    self->m_keyCallback(fromGLFWKey(key), act, mods);
  }
}

void GLFWMainWindow::mouseButtonClickCallback(GLFWwindow *window, int button,
                                              int action, int mods) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  if (self && self->m_mouseCallback) {
    double xpos, ypos, y;
    // getting cursor position
    glfwGetCursorPos(window, &xpos, &y);

    MouseButton mb = (button == GLFW_MOUSE_BUTTON_LEFT) ? MouseButton::Left
                                                        : MouseButton::Right;
    Action act = (action == GLFW_PRESS) ? Action::Press : Action::Release;

    self->m_mouseCallback(mb, act, mods, xpos, y);
  }
}

void GLFWMainWindow::cursorPosCallback(GLFWwindow *window, double xpos,
                                       double ypos) {
  int width, nowHeight;
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  if (self && self->m_cursorCallback) {
    double xpos, ypos;
    // getting cursor position
    glfwGetCursorPos(window, &xpos, &ypos);
    self->m_cursorCallback(xpos, ypos);
  }
}
void GLFWMainWindow::errorHandlerCallback(int error, const char *description) {
  Logger::error_log(description);
}

void GLFWMainWindow::resizeCallback(GLFWwindow *window, int width, int height) {
  auto *self = static_cast<GLFWMainWindow *>(glfwGetWindowUserPointer(window));
  self->config.width = width;
  self->config.height = height;

  if(self->m_resizeCallback) {
    self->m_resizeCallback(width, height);
  }
}

void *GLFWMainWindow::getNativeWindow() const { return m_window; }

WindowConfig GLFWMainWindow::getConfig() { return config; }

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
  // ... остальные клавиши
  default:
    return Key::Escape; // fallback
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
  // ... остальные клавиши
  default:
    return GLFW_KEY_UNKNOWN;
  }
}

} // namespace Window
