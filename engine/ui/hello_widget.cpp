#include "hello_widget.h"
#include <imgui/imgui.h> // Нужен для вызовов ImGui::

namespace ssme {

// В конструкторе задаем значения по умолчанию для нашего состояния
HelloWidget::HelloWidget()
    : m_show_demo_window(true), m_show_another_window(false),
      m_slider_value(0.0f), m_button_counter(0),
      m_clear_color(ImVec4(0.45f, 0.55f, 0.60f, 1.00f)) {}

void HelloWidget::render() {
  // Это тот самый код, который вы предоставили,
  // но теперь он использует переменные-члены класса (m_..._).
  ImGuiIO &io = ImGui::GetIO();

  // 1. Основное окно "Hello, world!"
  ImGui::Begin("Hello, world!");

  ImGui::Text("This is some useful text.");
  ImGui::Checkbox("Demo Window", &m_show_demo_window);
  ImGui::Checkbox("Another Window", &m_show_another_window);

  ImGui::SliderFloat("float", &m_slider_value, 0.0f, 1.0f);
  ImGui::ColorEdit3("clear color", (float *)&m_clear_color);

  if (ImGui::Button("Button"))
    m_button_counter++;
  ImGui::SameLine();
  ImGui::Text("counter = %d", m_button_counter);

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / io.Framerate, io.Framerate);
  ImGui::End();

  // 2. Опционально показываем другие окна, если включены галочки
  // if (m_show_demo_window)
  //     ImGui::ShowDemoWindow(&m_show_demo_window);

  if (m_show_another_window) {
    ImGui::Begin("Another Window", &m_show_another_window);
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me"))
      m_show_another_window = false;
    ImGui::End();
  }
}

} // namespace ssme
