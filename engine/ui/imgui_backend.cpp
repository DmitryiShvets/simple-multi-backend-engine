#include "imgui_backend.h"
#include "main_window.h"

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_dx12.h>
#include <imgui/imgui.h>

#include <GLFW/glfw3.h>

namespace ssme {
// ============================================================================
// ImGui Base
// ============================================================================

ImGuiBackend::ImGuiBackend() { m_context = ImGui::CreateContext(); }
ImGuiBackend::~ImGuiBackend() {
  if (!m_initialized) {
    return;
  }
  ImGui::SetCurrentContext(m_context);
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(m_context);
  m_initialized = false;
  m_window = nullptr;
  m_draw_data = nullptr;
}
void ImGuiBackend::configureContext(const UIBackendConfig &config) {
  IMGUI_CHECKVERSION();
  ImGuiIO &io = ImGui::GetIO();

  if (config.enable_keyboard) {
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  }
  if (config.enable_gamepad) {
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  }

  ImGui::StyleColorsDark();

  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(config.scale);
}
void ImGuiBackend::init(MainWindow &window,
                        const UIBackendConfig &config) {
  m_window = &window;
  m_glfw_window = static_cast<GLFWwindow *>(window.getNativeWindow());
  ImGui::SetCurrentContext(m_context);
  configureContext(config);
}

// ============================================================================
// ImGuiOpenGLBackend
// ============================================================================

void ImGuiOpenGLBackend::init(MainWindow &window,
                              const UIBackendConfig &config) {
  if (m_initialized) {
    return;
  }
  ImGuiBackend::init(window, config);
  ImGui_ImplGlfw_InitForOpenGL(m_glfw_window, false);
  window.setUiContext(m_context);
  m_initialized = true;
}

void ImGuiOpenGLBackend::frame() {
  if (!m_initialized) {
    return;
  }
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
}

// ============================================================================
// ImGuiVulkanBackend
// ============================================================================

void ImGuiVulkanBackend::init(MainWindow &window,
                              const UIBackendConfig &config) {
  if (m_initialized) {
    return;
  }
  ImGuiBackend::init(window, config);
  ImGui_ImplGlfw_InitForVulkan(m_glfw_window, false);
  window.setUiContext(m_context);
  m_initialized = true;
}

void ImGuiVulkanBackend::frame() {
  if (!m_initialized) {
    return;
  }
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
}


// ============================================================================
// ImGuiDirectX12Backend
// ============================================================================

void ImGuiDirectX12Backend::init(MainWindow &window,
                              const UIBackendConfig &config) {
  if (m_initialized) {
    return;
  }
  // ImGuiBackend::init(window, config);
  // ImGui_ImplGlfw_InitForVulkan(m_glfw_window, false);
  // window.setUiContext(m_context);
  // m_initialized = true;
}

void ImGuiDirectX12Backend::frame() {
  if (!m_initialized) {
    return;
  }
  // ImGui_ImplVulkan_NewFrame();
  // ImGui_ImplGlfw_NewFrame();
}

} // namespace ssme
