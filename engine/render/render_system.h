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
class ResourceManager;
/**
 * @brief Facade over multiple-backend renderers
 *
 */
class RenderSystem {
public:
  RenderSystem(Platform *platform, ResourceManager* rm ) : m_platform(platform), m_rm(rm) {}
  RenderSystem(const RenderSystem &) = delete;
  RenderSystem &operator=(const RenderSystem &) = delete;

  /**
   * @brief Initialize all renderers
   */
  void init(const std::vector<ImGuiContext *> &contexts);

  /**
   * @brief Rendering a frame to all renderers
   */
  void render(std::vector<SceneView> &scenes,
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
  const IRenderer &getRenderer(GpuBackend type) const;
  void addBackend(GpuBackend type);
  void createPerFrameResources();
  /**
   * @brief Get RenderDevice
   */
  RenderDevice &getDevice(GpuBackend type);
  void waitIdleAll();

private:
  Platform *m_platform;
  ResourceManager *m_rm;
  std::vector<std::unique_ptr<IRenderer>> m_renderers;
  std::shared_ptr<FrameData> m_frame_data = nullptr;
};

} // namespace ssme
