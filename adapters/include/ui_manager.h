#pragma once

#include "backend_type.h" // ← Core::BackendType
#include "imgui_backend.h"
#include <functional>
#include <memory>
#include <vector>

struct ImDrawData;

namespace Window {
class IMainWindow;
}

namespace UI {

/**
 * @brief Фасад для управления несколькими UI-бекендами
 */
class UIManager {
public:
  UIManager() = default;
  ~UIManager();

  UIManager(const UIManager &) = delete;
  UIManager &operator=(const UIManager &) = delete;

  void addBackend(std::unique_ptr<ImGuiBackend> backend);

  void
  init(const std::vector<std::reference_wrapper<Window::IMainWindow>> &windows,
       const UIBackendConfig &config = UIBackendConfig{});

  void render(const std::function<void()> &draw_fn);

  void destroy();

  size_t getBackendCount() const;

  // Доступ по типу бекенда
  ImGuiBackend &getBackend(Core::BackendType type);
  const ImGuiBackend &getBackend(Core::BackendType type) const;

  ImDrawData *getDrawData(Core::BackendType type) const;
  std::vector<ImDrawData *> getBundleDrawData() const;

  ImGuiContext *getContext(Core::BackendType type) const;

private:
  std::vector<std::unique_ptr<ImGuiBackend>> m_backends;
};

} // namespace UI
