#pragma once
#include <memory>

// Forward declarations
namespace Window {
class WindowManager;
}
namespace Render {
class RenderManager;
}
namespace UI {
class UIManager;
}
#include "flecs_world.h"
#include "world.h"

class Application {
public:
  void init();
  void run();
  void close();
  ~Application();

  Application() = delete;
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  Application(std::unique_ptr<Window::WindowManager> window_manager,
              std::unique_ptr<Render::RenderManager> render_manager,
              std::unique_ptr<UI::UIManager> ui_manager);

private:
  // Window & Render managers
  std::unique_ptr<Window::WindowManager> m_window_manager;
  std::unique_ptr<Render::RenderManager> m_render_manager;
  // UI
  std::unique_ptr<UI::UIManager> m_ui_manager;
  // ECS
  std::unique_ptr<Core::Ecs::World<Core::Ecs::FlecsWorldImpl>> m_world;
};
