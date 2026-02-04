#pragma once
#include <memory>

// Forward declarations
namespace Window {
class IMainWindow;
}
namespace Render {
class IRenderer;
} // namespace Render
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

  // The constructor now accepts two abstract IRenderer pointers
  Application(std::unique_ptr<Window::IMainWindow> gl_window,
              std::unique_ptr<Window::IMainWindow> vk_window,
              std::unique_ptr<Render::IRenderer> gl_renderer,
              std::unique_ptr<Render::IRenderer> vk_renderer);

private:
  std::unique_ptr<Window::IMainWindow> m_gl_window;
  std::unique_ptr<Window::IMainWindow> m_vk_window;

  // Both renderers are now represented by the same abstract interface
  std::unique_ptr<Render::IRenderer> m_gl_renderer;
  std::unique_ptr<Render::IRenderer> m_vulkan_renderer;

  // ECS World and Systems
  std::unique_ptr<Core::Ecs::World<Core::Ecs::FlecsWorldImpl>> m_world;
  std::unique_ptr<Core::Ecs::System::RenderSystem<
      Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>>
      m_render_system;
};
