#include "render_manager.h"
#include "scene_view.h"
#include <imgui.h>
#include <stdexcept>

namespace Render {

void RenderManager::addRenderer(Core::BackendType type, std::unique_ptr<IRenderer> renderer) {
    const size_t index = static_cast<size_t>(type);
    
    // Расширяем вектор если нужно
    if (index >= m_renderers.size()) {
        m_renderers.resize(index + 1);
    }
    
    m_renderers[index] = std::move(renderer);
}

void RenderManager::init(const std::vector<ImGuiContext*>& contexts) {
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

void RenderManager::frame(const std::vector<Core::SceneView>& scenes,
                          const std::vector<ImDrawData*>& ui_draw_data) {
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

void RenderManager::destroy() {
    for (auto& renderer : m_renderers) {
        if (renderer) {
            renderer->destroy();
        }
    }
    m_renderers.clear();
}

size_t RenderManager::getRendererCount() const {
    return m_renderers.size();
}

IRenderer& RenderManager::getRenderer(Core::BackendType type) {
    const size_t index = static_cast<size_t>(type);
    
    if (index >= m_renderers.size() || !m_renderers[index]) {
        throw std::out_of_range("Renderer not found for backend type");
    }
    return *m_renderers[index];
}

const IRenderer& RenderManager::getRenderer(Core::BackendType type) const {
    const size_t index = static_cast<size_t>(type);
    
    if (index >= m_renderers.size() || !m_renderers[index]) {
        throw std::out_of_range("Renderer not found for backend type");
    }
    return *m_renderers[index];
}

RenderDevice& RenderManager::getDevice(Core::BackendType type) {
    return getRenderer(type).getRenderDeivce();
}

} // namespace Render
