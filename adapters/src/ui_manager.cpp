#include "ui_manager.h"
#include "i_main_window.h" // Нужен для получения GLFWwindow*

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_vulkan.h>

namespace UI {
UIManager::UIManager() {}
UIManager::~UIManager() {}

void UIManager::configureNewContext() {
  IMGUI_CHECKVERSION();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui::StyleColorsDark();

  const float main_scale = 1.0f;
  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
}

void UIManager::init(Window::IMainWindow &wnd_vulkan,
                     Window::IMainWindow &wnd_opengl) {
  auto glfw_window_vk = static_cast<GLFWwindow *>(wnd_vulkan.getNativeWindow());
  auto glfw_window_gl = static_cast<GLFWwindow *>(wnd_opengl.getNativeWindow());
  // --- Настройка контекста для VULKAN ---
  m_ctx_vulkan = ImGui::CreateContext();
  ImGui::SetCurrentContext(m_ctx_vulkan); // Устанавливаем текущий контекст
  configureNewContext();                  // Применяем общие настройки
  ImGui_ImplGlfw_InitForVulkan(glfw_window_vk, false);
  wnd_vulkan.setUiContext(m_ctx_vulkan);
  // --- Настройка контекста для OPENGL ---
  m_ctx_opengl = ImGui::CreateContext();
  ImGui::SetCurrentContext(m_ctx_opengl); // Устанавливаем текущий контекст
  configureNewContext();                  // Применяем те же настройки
  ImGui_ImplGlfw_InitForOpenGL(glfw_window_gl, false);
  wnd_opengl.setUiContext(m_ctx_opengl);
}

void UIManager::render(const std::function<void()> &draw_vulkan_ui,
                       const std::function<void()> &draw_opengl_ui) {
  ImGui::SetCurrentContext(m_ctx_vulkan);
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  draw_vulkan_ui();
  ImGui::Render();
  m_draw_data_vulkan = ImGui::GetDrawData();

  ImGui::SetCurrentContext(m_ctx_opengl);
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  draw_opengl_ui();
  ImGui::Render();
  m_draw_data_opengl = ImGui::GetDrawData();
}

void UIManager::destroy() {
  // Важно корректно завершить работу и уничтожить оба контекста

  // Завершаем сессию для Vulkan
  ImGui::SetCurrentContext(m_ctx_vulkan);
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(m_ctx_vulkan);

  // Завершаем сессию для OpenGL
  ImGui::SetCurrentContext(m_ctx_opengl);
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(m_ctx_opengl);
}

ImDrawData *UIManager::getVulkanDrawData() const { return m_draw_data_vulkan; }
ImDrawData *UIManager::getOpenGLDrawData() const { return m_draw_data_opengl; }

ImGuiContext *UIManager::getVulkanContext() const { return m_ctx_vulkan; }
ImGuiContext *UIManager::getOpenGLContext() const { return m_ctx_opengl; }

} // namespace UI
