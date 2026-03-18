#pragma once

// Включаем imgui.h, так как мы используем его типы, например ImVec4
#include <imgui/imgui.h>

namespace ssme {

// Простой класс-виджет, который инкапсулирует одно окно ImGui
class HelloWidget {
public:
  // Конструктор, задает начальные значения
  HelloWidget();

  // Метод, который нужно вызывать каждый кадр для отрисовки окна
  void render();

  float m_slider_value;

private:
  // Состояние виджетов теперь хранится здесь, а не в статических переменных
  bool m_show_demo_window;
  bool m_show_another_window;
  int m_button_counter;
  ImVec4 m_clear_color;
};

} // namespace ssme
