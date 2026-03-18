#include "ui_manager.h"
#include "main_window.h"
#include <imgui/imgui.h>
#include <stdexcept>

namespace ssme {

UIManager::~UIManager() { destroy(); }

void UIManager::addBackend(std::unique_ptr<ImGuiBackend> backend) {
  m_backends.push_back(std::move(backend));
}

void UIManager::addBackend(GpuBackend type) {
  switch (type) {
  case GpuBackend::OpenGL:
    m_backends.push_back(std::make_unique<ImGuiOpenGLBackend>());
    break;
  case GpuBackend::Vulkan:
    m_backends.push_back(std::make_unique<ImGuiVulkanBackend>());
    break;
  default:
    throw std::runtime_error("Unknown backend type");
  }
}

void UIManager::init(
    const std::vector<std::reference_wrapper<MainWindow>> &windows,
    const UIBackendConfig &config) {
  if (windows.size() != m_backends.size()) {
    throw std::invalid_argument(
        "Number of windows must match number of backends");
  }

  for (size_t i = 0; i < m_backends.size(); ++i) {
    m_backends[i]->init(windows[i].get(), config);
  }
}

void UIManager::render(const std::function<void()> &draw_fn) {
  for (auto &backend : m_backends) {
    // Switch to this backend's context
    ImGui::SetCurrentContext(backend->getContext());
    backend->frame();
    ImGui::NewFrame();
    draw_fn();
    ImGui::Render();
    auto data = ImGui::GetDrawData();
    backend->setDrawData(data);
  }
}

void UIManager::destroy() { m_backends.clear(); }

size_t UIManager::getBackendCount() const { return m_backends.size(); }

ImGuiBackend &UIManager::getBackend(GpuBackend type) {
  const size_t index = static_cast<size_t>(type);
  if (index >= m_backends.size()) {
    throw std::out_of_range("Backend index out of range");
  }
  return *m_backends[index];
}

const ImGuiBackend &UIManager::getBackend(GpuBackend type) const {
  const size_t index = static_cast<size_t>(type);
  if (index >= m_backends.size()) {
    throw std::out_of_range("Backend index out of range");
  }
  return *m_backends[index];
}

std::vector<ImDrawData *> UIManager::getBundleDrawData() const {
  std::vector<ImDrawData *> result;
  for (auto &ui : m_backends) {
    result.push_back(ui->getDrawData());
  }
  return result;
}

ImDrawData *UIManager::getDrawData(GpuBackend type) const {
  const size_t index = static_cast<size_t>(type);
  if (index >= m_backends.size()) {
    return nullptr;
  }
  return m_backends[index]->getDrawData();
}

ImGuiContext *UIManager::getContext(GpuBackend type) const {
  const size_t index = static_cast<size_t>(type);
  if (index >= m_backends.size()) {
    return nullptr;
  }
  return m_backends[index]->getContext();
}

} // namespace ssme
