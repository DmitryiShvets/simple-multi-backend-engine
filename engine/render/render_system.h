#pragma once
#include "core/gpu_types.h"
#include "render_device.h"
#include "renderer.h"
#include <memory>
#include <vector>

// Forward declare ImGui types
struct ImGuiContext;
struct ImDrawData;

namespace ssme {
class SceneView;
class Platform;

/**
 * @brief Facade over multiple-backend renderers
 *
 */
class RenderSystem {
public:
  RenderSystem(Platform *platform) : m_platform(platform) {}
  RenderSystem(const RenderSystem &) = delete;
  RenderSystem &operator=(const RenderSystem &) = delete;

  /**
   * @brief Initialize all renderers
   */
  void init(const std::vector<ImGuiContext *> &contexts);

  /**
   * @brief Rendering a frame to all renderers
   */
  void render(const std::vector<SceneView> &scenes,
              const std::vector<ImDrawData *> &ui_draw_data);

  /**
   * @brief Cleanup of resources
   */
  void destroy();

  /**
   * @brief Get count of renderers
   */
  size_t getRendererCount() const;

  /**
   * @brief Get render by it type
   */
  IRenderer &getRenderer(GpuBackend type);
  IRenderer &getRenderer(size_t type);
  const IRenderer &getRenderer(GpuBackend type) const;
  void addBackend(GpuBackend type);

  /**
   * @brief Get RenderDevice
   */
  RenderDevice &getDevice(GpuBackend type);

  void waitIdle(size_t index);

  void waitIdleAll();

private:
  Platform *m_platform;
  std::vector<std::unique_ptr<IRenderer>> m_renderers;
};

} // namespace ssme
