#pragma once

#include <imgui/imgui.h>

namespace ssme {

class ActionBus;

/**
 * @brief Simple widget class that encapsulates an ImGui window
 */
class HelloWidget {
public:
  HelloWidget();

  /**
   * @brief Call this method every frame to render the window
   */
  void render();

  float m_slider_value;
  void setActionBus(ActionBus *bus) { m_action_bus = bus; }
private:
  bool m_show_demo_window;
  bool m_show_another_window;
  int m_button_counter;
  ImVec4 m_clear_color;
  ActionBus *m_action_bus = nullptr;
};

} // namespace ssme
