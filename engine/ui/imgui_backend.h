#pragma once

#include "main_window.h"
#include <string>

struct GLFWwindow;
struct ImGuiContext;
struct ImDrawData;

namespace ssme {

/**
 * @brief UI backend configuration
 */
struct UIBackendConfig {
  std::string name;
  bool enable_keyboard = true;
  bool enable_gamepad = true;
  float scale = 1.0f;
};

/**
 * @brief Base RAII wrapper class for ImGui backend
 */
class ImGuiBackend {
public:
  ImGuiBackend();
  virtual ~ImGuiBackend();

  virtual void init(MainWindow &window, const UIBackendConfig &config);
  virtual void frame() = 0;
  void configureContext(const UIBackendConfig &config);

  ImGuiContext *getContext() { return m_context; }
  ImDrawData *getDrawData() { return m_draw_data; }
  void setDrawData(ImDrawData *data) { m_draw_data = data; }
  MainWindow &getWindow() { return *m_window; }

protected:
  ImGuiContext *m_context = nullptr;
  ImDrawData *m_draw_data = nullptr;
  bool m_initialized = false;
  MainWindow *m_window = nullptr;
  GLFWwindow *m_glfw_window = nullptr;
};

/**
 * @brief RAII wrapper for OpenGL ImGui backend
 */
class ImGuiOpenGLBackend : public ImGuiBackend {
public:
  void init(MainWindow &window,
            const UIBackendConfig &config) override;
  void frame() override;
};

/**
 * @brief RAII wrapper for Vulkan ImGui backend
 */
class ImGuiVulkanBackend : public ImGuiBackend {
public:
  void init(MainWindow &window,
            const UIBackendConfig &config) override;
  void frame() override;
};

} // namespace ssme
