#pragma once
#include <memory>

// Forward declarations
namespace Window {
class IMainWindow;
}
namespace Render {
class IRenderer;
}
namespace UI {
class UIManager;
}
// namespace Core::Ecs {
// template <typename T> class World;
// class FlecsWorldImpl;
// }
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

  Application(std::unique_ptr<Window::IMainWindow> gl_window,
              std::unique_ptr<Window::IMainWindow> vk_window,
              std::unique_ptr<Render::IRenderer> gl_renderer,
              std::unique_ptr<Render::IRenderer> vk_renderer,
              std::unique_ptr<UI::UIManager> ui_manager);

private:
  // Windows
  std::unique_ptr<Window::IMainWindow> m_gl_window;
  std::unique_ptr<Window::IMainWindow> m_vk_window;
  // Renderers
  std::unique_ptr<Render::IRenderer> m_gl_renderer;
  std::unique_ptr<Render::IRenderer> m_vk_renderer;
  // UI
  std::unique_ptr<UI::UIManager> m_ui_manager;
  // ECS
  std::unique_ptr<Core::Ecs::World<Core::Ecs::FlecsWorldImpl>> m_world;
};
