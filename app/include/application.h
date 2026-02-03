#pragma once
#include <memory>

// Forward declarations
namespace Window {
class IMainWindow;
}
namespace Render {
class IRenderer;
class RenderingDevice;
} // namespace Render

// ECS includes
#include "ecs/systems/render_system.h"
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

  Application(std::unique_ptr<Window::IMainWindow> window,
              std::unique_ptr<Render::IRenderer> renderer,
              std::unique_ptr<Render::RenderingDevice> rd);

private:
  std::unique_ptr<Window::IMainWindow> m_window;
  std::unique_ptr<Render::IRenderer> m_renderer;
  std::unique_ptr<Render::RenderingDevice> m_rendering_device;
  std::unique_ptr<Core::Ecs::World<Core::Ecs::FlecsWorldImpl>> m_world;
  std::unique_ptr<Core::Ecs::System::RenderSystem<
      Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>>
      m_render_system;
};
