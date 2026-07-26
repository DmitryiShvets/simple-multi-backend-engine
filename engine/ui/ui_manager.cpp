#include "ui_manager.h"
#include "imgui_backend.h"
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
  case GpuBackend::DirectX12:
    m_backends.push_back(std::make_unique<ImGuiDirectX12Backend>());
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
    m_backends[i]->init(windows[i].get(), config,
                        windows[i].get().getGpuBackend());
  }
}

void UIManager::render(const std::function<void()> &draw_fn) {
  for (auto &backend : m_backends) {
    if (!backend->isInitialized())
      continue;
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
  auto backend =
      std::find_if(m_backends.begin(), m_backends.end(),
                   [type](auto &e) { return e->getGpuBackend() == type; });
  if (backend == m_backends.end()) {
    throw std::out_of_range("Backend index out of range");
  }
  return **backend;
}

const ImGuiBackend &UIManager::getBackend(GpuBackend type) const {
  auto backend =
      std::find_if(m_backends.begin(), m_backends.end(),
                   [type](auto &e) { return e->getGpuBackend() == type; });
  if (backend == m_backends.end()) {
    throw std::out_of_range("Backend index out of range");
  }
  return **backend;
}

std::vector<ImDrawData *> UIManager::getBundleDrawData() const {
  std::vector<ImDrawData *> result;
  for (auto &ui : m_backends) {
    result.push_back(ui->getDrawData());
  }
  return result;
}

ImDrawData *UIManager::getDrawData(GpuBackend type) const {
  auto backend =
      std::find_if(m_backends.begin(), m_backends.end(),
                   [type](auto &e) { return e->getGpuBackend() == type; });
  if (backend == m_backends.end()) {
    throw std::out_of_range("Backend index out of range");
  }
  return (*backend)->getDrawData();
}

ImGuiContext *UIManager::getContext(GpuBackend type) const {
  auto backend =
      std::find_if(m_backends.begin(), m_backends.end(),
                   [type](auto &e) { return e->getGpuBackend() == type; });
  if (backend == m_backends.end()) {
    throw std::out_of_range("Backend index out of range");
  }
  return (*backend)->getContext();
}

} // namespace ssme
