#include "application.h"
#include "i_main_window.h"
#include "i_renderer.h"

#include "objects_utils.h"
#include "runtime/runtime_init_system.h"
#include "scene_view.h"
#include "sphere.h"
#include "vertex.h"

#include "flecs_world.h"
#include "world.h"

#include "ecs/systems/scene_view_system.h"

#include <chrono>
#include <memory>
#include <vector>

// The constructor now matches the new header, accepting two abstract renderers
Application::Application(std::unique_ptr<Window::IMainWindow> gl_window,
                         std::unique_ptr<Window::IMainWindow> vk_window,
                         std::unique_ptr<Render::IRenderer> gl_renderer,
                         std::unique_ptr<Render::IRenderer> vk_renderer)
    : m_gl_window(std::move(gl_window)), m_vk_window(std::move(vk_window)),
      m_gl_renderer(std::move(gl_renderer)),
      m_vk_renderer(std::move(vk_renderer)) {}

Application::~Application() = default;

void Application::init() {

  m_gl_window->setPosition(100, 100);
  m_vk_window->setPosition(950, 100);

  m_world = std::make_unique<Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>();

  // Create test objects in the ECS
  // auto triangle = m_world->createEntity();
  auto sphere = getSphere3D(1, 16, 16);
  std::vector<Vertex> pos1;
  int j = 1;
  for (auto i : sphere.indices) {

    pos1.push_back({sphere.positions[i],
                    {j % 3 == 1 * 1.0f, j % 3 == 2 * 1.0f, j % 3 == 0 * 1.0f}});
    j++;
  }
  auto pos2 = std::vector<Vertex>{
      {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Colors don't matter, the debug shader will override them
      {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
  };
  auto pos = std::vector<Vertex>{
      {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // pos, color
  };
  auto triangle = createMesh(*m_world, pos1, "default");
  // auto triangle1 = createMesh(*m_world, pos2, "default");

  // --- Initialize runtime resources ---
  // This system queries for entities with Geometry and Material and creates the
  // corresponding GPU resources for each backend.
  auto runtime_init_system =
      std::make_unique<Core::Ecs::System::RuntimeInitSystem<
          Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>>(
          *m_world, m_vk_renderer->getRenderDeivce(),
          m_gl_renderer->getRenderDeivce());
  runtime_init_system->initialize();
}

void Application::run() {
  // Create a specific scene view system for each renderer
  auto vk_scene_view_system =
      std::make_unique<Core::Ecs::System::SceneViewSystem<
          Core::Ecs::World<Core::Ecs::FlecsWorldImpl>,
          Core::Ecs::Component::VkRuntime>>(*m_world);

  auto gl_scene_view_system =
      std::make_unique<Core::Ecs::System::SceneViewSystem<
          Core::Ecs::World<Core::Ecs::FlecsWorldImpl>,
          Core::Ecs::Component::GlRuntime>>(*m_world);

  auto lastTime = std::chrono::high_resolution_clock::now();

  // The main loop is now extremely simple and clean.
  while (!m_gl_window->shouldClose() &&
         !m_vk_window->shouldClose()) { // Assuming one window controls the
                                        // lifecycle for now
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime =
        std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    m_gl_window->pollEvents();
    m_vk_window->pollEvents();

    // A single, abstract call to each renderer, passing a view with the
    // correct RIDs for that renderer.
    if (m_gl_renderer) {
      Core::SceneView gl_scene_view = gl_scene_view_system->run();
      m_gl_renderer->renderFrame(gl_scene_view);
    }
    if (m_vk_renderer) {
      Core::SceneView vk_scene_view = vk_scene_view_system->run();
      m_vk_renderer->renderFrame(vk_scene_view);
    }
    m_gl_window->swapBuffers();
  }
}

void Application::close() {
  if (m_gl_window) {
    m_gl_window->destroy();
  }
  if (m_vk_window) {
    m_vk_window->destroy();
  }
}
