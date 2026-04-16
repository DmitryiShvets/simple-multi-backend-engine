#pragma once

#include <imgui/imgui.h>

namespace ssme {

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

private:
  bool m_show_demo_window;
  bool m_show_another_window;
  int m_button_counter;
  ImVec4 m_clear_color;
};

} // namespace ssme
