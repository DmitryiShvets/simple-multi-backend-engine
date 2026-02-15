#pragma once

#include "hello_widget.h"
#include <functional>

struct ImGuiContext;
struct ImDrawData;
namespace Window {
class IMainWindow;
}

namespace UI {
class UIManager {
public:
  UIManager();
  ~UIManager();

  void init(Window::IMainWindow &wnd_vulkan, Window::IMainWindow &wnd_opengl);
  void render(const std::function<void()> &draw_vulkan_ui,
              const std::function<void()> &draw_opengl_ui);
  void destroy();
  ImDrawData *getVulkanDrawData() const;
  ImDrawData *getOpenGLDrawData() const;
  ImGuiContext *getVulkanContext() const;
  ImGuiContext *getOpenGLContext() const;

private:
  void configureNewContext();

  ImGuiContext *m_ctx_vulkan = nullptr;
  ImGuiContext *m_ctx_opengl = nullptr;

  ImDrawData *m_draw_data_vulkan = nullptr;
  ImDrawData *m_draw_data_opengl = nullptr;

  UI::HelloWidget widget;
};
} // namespace UI
