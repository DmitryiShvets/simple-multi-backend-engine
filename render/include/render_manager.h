#pragma once
#include "i_renderer.h"
#include "render_device.h"
#include "backend_type.h"  // ← Core::BackendType
#include <memory>
#include <vector>
#include <stdexcept>

// Forward declare ImGui types
struct ImGuiContext;
struct ImDrawData;

namespace Core {
class SceneView;
}

namespace Render {

/**
 * @brief Фасад над несколькими рендерерами
 * 
 * Управляет коллекцией рендереров без привязки к конкретным API
 */
class RenderManager {
public:
    RenderManager() = default;
    RenderManager(const RenderManager&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;

    /**
     * @brief Добавить рендерер в менеджер
     */
    void addRenderer(Core::BackendType type, std::unique_ptr<IRenderer> renderer);

    /**
     * @brief Инициализировать все рендереры
     */
    void init(const std::vector<ImGuiContext*>& contexts);

    /**
     * @brief Рендеринг кадра во все рендереры
     */
    void frame(const std::vector<Core::SceneView>& scenes,
               const std::vector<ImDrawData*>& ui_draw_data);

    /**
     * @brief Очистка ресурсов
     */
    void destroy();

    /**
     * @brief Получить количество рендереров
     */
    size_t getRendererCount() const;

    /**
     * @brief Получить рендерер по типу
     */
    IRenderer& getRenderer(Core::BackendType type);
    const IRenderer& getRenderer(Core::BackendType type) const;

    /**
     * @brief Получить RenderDevice рендерера
     */
    RenderDevice& getDevice(Core::BackendType type);

private:
    std::vector<std::unique_ptr<IRenderer>> m_renderers;
};

} // namespace Render
