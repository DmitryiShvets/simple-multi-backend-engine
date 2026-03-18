#include "render_system.h"
#include "core/gpu_types.h"
#include "core/scene_view.h"
#include "opengl_renderer.h"
#include "vulkan_renderer.h"
#include <cstddef>
#include <imgui.h>
#include <stdexcept>

namespace ssme {
void RenderSystem::addBackend(GpuBackend type) {
  switch (type) {
  case GpuBackend::OpenGL:
    m_renderers.push_back(std::make_unique<OpenGLRenderer>(m_platform));
    break;
  case GpuBackend::Vulkan:
    m_renderers.push_back(std::make_unique<VulkanRenderer>(m_platform));
    break;
  default:
    throw std::runtime_error("Unknown backend type");
  }
}

void RenderSystem::init(const std::vector<ImGuiContext *> &contexts) {
  if (contexts.size() != m_renderers.size()) {
    throw std::invalid_argument(
        "Number of contexts must match number of renderers");
  }
  for (size_t i = 0; i < m_renderers.size(); ++i) {
    if (m_renderers[i]) {
      m_renderers[i]->init(contexts[i]);
    }
  }
}

void RenderSystem::render(const std::vector<SceneView> &scenes,
                          const std::vector<ImDrawData *> &ui_draw_data) {
  if (scenes.size() != m_renderers.size()) {
    throw std::invalid_argument(
        "Number of scenes must match number of renderers");
  }

  if (ui_draw_data.size() != m_renderers.size()) {
    throw std::invalid_argument(
        "Number of UI draw data must match number of renderers");
  }

  for (size_t i = 0; i < m_renderers.size(); ++i) {
    if (m_renderers[i]) {
      m_renderers[i]->renderFrame(scenes[i], ui_draw_data[i]);
    }
  }
}

void RenderSystem::destroy() {
  for (auto &renderer : m_renderers) {
    if (renderer) {
      renderer->destroy();
    }
  }
  m_renderers.clear();
}

size_t RenderSystem::getRendererCount() const { return m_renderers.size(); }

IRenderer &RenderSystem::getRenderer(GpuBackend type) {
  const size_t index = static_cast<size_t>(type);

  if (index >= m_renderers.size() || !m_renderers[index]) {
    throw std::out_of_range("Renderer not found for backend type");
  }
  return *m_renderers[index];
}

IRenderer &RenderSystem::getRenderer(size_t index) {
  if (index >= m_renderers.size() || !m_renderers[index]) {
    throw std::out_of_range("Renderer not found for backend type");
  }
  return *m_renderers[index];
}

const IRenderer &RenderSystem::getRenderer(GpuBackend type) const {
  const size_t index = static_cast<size_t>(type);

  if (index >= m_renderers.size() || !m_renderers[index]) {
    throw std::out_of_range("Renderer not found for backend type");
  }
  return *m_renderers[index];
}

RenderDevice &RenderSystem::getDevice(GpuBackend type) {
  return getRenderer(type).getRenderDeivce();
}

void RenderSystem::waitIdle(size_t index) { getRenderer(index).waitIdle(); }

void RenderSystem::waitIdleAll() {
  for (auto &renderer : m_renderers) {
    renderer->waitIdle();
  }
}

} // namespace ssme
