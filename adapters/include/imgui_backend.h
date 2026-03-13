#pragma once

#include "i_main_window.h"
#include <string>

class GLFWwindow;
class ImGuiContext;
class ImDrawData;

namespace UI {

/**
 * @brief Конфигурация UI-бекенда
 */
struct UIBackendConfig {
  std::string name;
  bool enable_keyboard = true;
  bool enable_gamepad = true;
  float scale = 1.0f;
};

/**
 * @brief Базовый класс RAII-обертки для ImGui-бекенда
 */
class ImGuiBackend {
public:
  ImGuiBackend();
  virtual ~ImGuiBackend();

  virtual void init(Window::IMainWindow &window, const UIBackendConfig &config);
  virtual void frame() = 0;
  void configureContext(const UIBackendConfig &config);

  ImGuiContext *getContext() { return m_context; }
  ImDrawData *getDrawData() { return m_draw_data; }
  void setDrawData(ImDrawData *data) { m_draw_data = data; }
  Window::IMainWindow &getWindow() { return *m_window; }

protected:
  ImGuiContext *m_context = nullptr;
  ImDrawData *m_draw_data = nullptr;
  bool m_initialized = false;
  Window::IMainWindow *m_window = nullptr;
  GLFWwindow *m_glfw_window = nullptr;
};

/**
 * @brief RAII-обертка для OpenGL ImGui-бекенда
 */
class ImGuiOpenGLBackend : public ImGuiBackend {
public:
  void init(Window::IMainWindow &window, const UIBackendConfig &config) override;
  void frame() override;
};

/**
 * @brief RAII-обертка для Vulkan ImGui-бекенда
 */
class ImGuiVulkanBackend : public ImGuiBackend {
public:
  void init(Window::IMainWindow &window, const UIBackendConfig &config) override;
  void frame() override;
};

} // namespace UI
