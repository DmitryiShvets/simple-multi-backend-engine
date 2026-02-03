#include "application.h"
#include "i_main_window.h"
#include "i_renderer.h"

// Include the concrete world implementation
#include "flecs_world.h"
#include "render_types.h"
#include "rendering_device.h"
#include "world.h"

// Include components that we will use
#include "ecs/components/render_component.h"
#include "ecs/components/transform_component.h"

#include "ecs/systems/render_system.h"
#include <chrono>
#include <memory>
using World = Core::Ecs::World<Core::Ecs::FlecsWorldImpl>;

void Application::init() {
  auto cfg = m_window->getConfig();
  m_renderer->initialize(cfg.width, cfg.height);
  // 1. Create our new ECS World
  m_world = std::make_unique<World>();

  // 2. Create the new RenderSystem, passing the world directly
  m_render_system = std::make_unique<Core::Ecs::System::RenderSystem<World>>(
      *m_world, *m_renderer);
  m_render_system->enableBatching(true);
  m_render_system->enableSorting(true);

  // 3. Recreate the test objects manually
  // Create a circle
  auto circle = m_world->createEntity();
  m_world->addComponent<Core::Ecs::Component::Transform>(
      circle, glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f));
  m_world->addComponent<Core::Ecs::Component::Renderable>(
      circle, "circle", "custom", glm::vec3(1.0f, 0.0f, 0.0f));

  // Create a square
  auto square = m_world->createEntity();
  m_world->addComponent<Core::Ecs::Component::Transform>(
      square, glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.2f));
  m_world->addComponent<Core::Ecs::Component::Renderable>(
      square, "quad", "custom", glm::vec3(1.0f, 0.0f, 1.0f));
}

void Application::run() {
  m_renderer->setClearColor(175.0f / 255.0f, 218.0f / 255.0f, 252.0f / 255.0f,
                            1.0f);
  auto lastTime = std::chrono::high_resolution_clock::now();
  Render::SwapChainDesc des{};
  Render::RID swap_chain_rid = m_rendering_device->createSwapChain(des);
  // App loop
  while (!m_window->shouldClose()) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime =
        std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    m_window->pollEvents();

    // 1. Получаем RID конкретного back buffer'а на текущий кадр
    Render::RID backBufferRid =
        m_rendering_device->acquireNextFrame(swap_chain_rid);
    Render::CommandList *cmdList =
        m_rendering_device->beginCommandList(swap_chain_rid);

    cmdList->begin();
    // 1. Transition layout from whatever it was to a layout optimal for transfer operations
    cmdList->resourceBarrier(backBufferRid, Render::ResourceState::UNDEFINED,
                             Render::ResourceState::TRANSFER_DST);

    // 2. Clear the resource
    float ff[] = {0.1f, 0.1f, 0.2f, 1.0f};
    cmdList->clearRenderTarget(backBufferRid, ff);

    // 3. Transition layout to be ready for presentation
    cmdList->resourceBarrier(backBufferRid, Render::ResourceState::TRANSFER_DST,
                             Render::ResourceState::PRESENT_SRC);
    cmdList->end();

    m_rendering_device->submitCommandList(swap_chain_rid, cmdList);

    m_renderer->clear();

    m_render_system->update(deltaTime);
    m_window->swapBuffers();
  }
  m_rendering_device->waitIdle(); // Force CPU-GPU sync

}

void Application::close() {
  m_renderer->destroy();
  m_window->destroy();
}

Application::~Application() = default;

Application::Application(std::unique_ptr<Window::IMainWindow> window,
                         std::unique_ptr<Render::IRenderer> renderer,
                         std::unique_ptr<Render::RenderingDevice> rd)
    : m_window(std::move(window)), m_renderer(std::move(renderer)),
      m_rendering_device(std::move(rd)) {}
