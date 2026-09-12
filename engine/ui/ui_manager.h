#pragma once

#include "core/gpu_types.h"
#include "imgui_backend.h"
#include <functional>
#include <memory>
#include <vector>

struct ImDrawData;

namespace Window {
class IMainWindow;
}

namespace ssme {

/**
 * @brief Facade for managing multiple UI backends
 */
class UIManager {
public:
  UIManager() = default;
  ~UIManager();

  UIManager(const UIManager &) = delete;
  UIManager &operator=(const UIManager &) = delete;

  void addBackend(std::unique_ptr<ImGuiBackend> backend);
  void addBackend(GpuBackend type);

  void init(const std::vector<std::reference_wrapper<MainWindow>> &windows,
            const UIBackendConfig &config = UIBackendConfig{});

  void render(const std::function<void()> &draw_fn);

  void destroy();

  size_t getBackendCount() const;

  // Access by backend type
  ImGuiBackend &getBackend(GpuBackend type);
  const ImGuiBackend &getBackend(GpuBackend type) const;

  ImDrawData *getDrawData(GpuBackend type) const;
  std::vector<ImDrawData *> getBundleDrawData() const;

  ImGuiContext *getContext(GpuBackend type) const;

private:
  std::vector<std::unique_ptr<ImGuiBackend>> m_backends;
};

} // namespace ssme
