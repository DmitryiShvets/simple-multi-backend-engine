#include "application.h"
#include "ecs/components/transform_component.h"
#include "ecs/components/material_component.h"
#include "hello_widget.h"
#include "i_main_window.h"
#include "i_renderer.h"

#include "objects_utils.h"
#include "runtime/runtime_init_system.h"
#include "scene_view.h"

#include "flecs_world.h"
#include "world.h"

#include "ecs/systems/scene_view_system.h"
#include "runtime/runtime_update_system.h"

#include "ui_manager.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

// The constructor now matches the new header, accepting two abstract renderers
Application::Application(std::unique_ptr<Window::IMainWindow> gl_window,
                         std::unique_ptr<Window::IMainWindow> vk_window,
                         std::unique_ptr<Render::IRenderer> gl_renderer,
                         std::unique_ptr<Render::IRenderer> vk_renderer,
                         std::unique_ptr<UI::UIManager> ui_manager)
    : m_gl_window(std::move(gl_window)), m_vk_window(std::move(vk_window)),
      m_gl_renderer(std::move(gl_renderer)),
      m_vk_renderer(std::move(vk_renderer)),
      m_ui_manager(std::move(ui_manager)) {}

Application::~Application() = default;

void Application::init() {

  m_gl_window->setPosition(100, 100);
  m_vk_window->setPosition(950, 100);

  m_world = std::make_unique<Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>();

  // Create test objects in the ECS
  // Create spheres using the sphere generator
  auto sphere1 = createSphere(*m_world, "sphere1", 0.5f, 32, 32, "ads");
  m_world->addComponent<Core::Ecs::Component::AdsMaterial>(
      sphere1, Core::Ecs::Component::AdsMaterial{
                    .color = glm::vec3(1.0f, 0.0f, 0.0f)});

  auto sphere2 = createSphere(*m_world, "sphere2", 0.6f, 32, 32, "ads");
  m_world->addComponent<Core::Ecs::Component::AdsMaterial>(
      sphere2, Core::Ecs::Component::AdsMaterial{
                    .color = glm::vec3(1.0f, 1.0f, 0.0f)});
  // --- Initialize runtime resources ---
  auto runtime_init_system =
      std::make_unique<Core::Ecs::System::RuntimeInitSystem<
          Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>>(
          *m_world, m_gl_renderer->getRenderDeivce(),
          m_vk_renderer->getRenderDeivce());
  runtime_init_system->initialize();
}

void Application::run() {
  // Create scene view systems for each renderer
  auto vk_scene_view_system =
      std::make_unique<Core::Ecs::System::SceneViewSystem<
          Core::Ecs::World<Core::Ecs::FlecsWorldImpl>,
          Core::Ecs::Component::VkRuntime>>(*m_world);

  auto gl_scene_view_system =
      std::make_unique<Core::Ecs::System::SceneViewSystem<
          Core::Ecs::World<Core::Ecs::FlecsWorldImpl>,
          Core::Ecs::Component::GlRuntime>>(*m_world);

  // Create single runtime update system for both backends
  auto runtime_update_system =
      std::make_unique<Core::Ecs::System::RuntimeUpdateSystem<
          Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>>(
          *m_world, m_vk_renderer->getRenderDeivce(),
          m_gl_renderer->getRenderDeivce());

  auto lastTime = std::chrono::high_resolution_clock::now();

  UI::HelloWidget widget;
  // The main loop is now extremely simple and clean.
  while (!m_gl_window->shouldClose() &&
         !m_vk_window->shouldClose()) { // Assuming one window controls the
                                        // lifecycle for now
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime =
        std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    m_gl_window->update();
    m_vk_window->update();

    m_ui_manager->render([&widget]() { widget.render(); },
                         [&widget]() { widget.render(); });
    // test only
    // auto triangle = m_world->getEntity("triangle");
    // auto &transform =
    //     m_world->getComponent<Core::Ecs::Component::Transform>(triangle);
    // transform.position.z = widget.m_slider_value;
    auto sphere1 = m_world->getEntity("sphere1");
     auto& transform1 = m_world->getComponent<Core::Ecs::Component::Transform>(sphere1);
     transform1.position.x = -widget.m_slider_value;
     // transform1.position.z = -1;
     auto sphere2 = m_world->getEntity("sphere2");
     auto& transform2 = m_world->getComponent<Core::Ecs::Component::Transform>(sphere2);
     transform2.position.x = widget.m_slider_value;
     // transform2.position.z = 1;
    // Update per-object uniform buffers before rendering
    runtime_update_system->update();

    Core::SceneView gl_scene_view = gl_scene_view_system->run();
    gl_scene_view.z = widget.m_slider_value -0.5;
    m_gl_renderer->renderFrame(gl_scene_view,
                               m_ui_manager->getOpenGLDrawData());

    Core::SceneView vk_scene_view = vk_scene_view_system->run();
    vk_scene_view.z = widget.m_slider_value -0.5;
    m_vk_renderer->renderFrame(vk_scene_view,
                               m_ui_manager->getVulkanDrawData());

    m_gl_window->swapBuffers();
  }
}

void Application::close() {
  m_gl_renderer->destroy();
  m_vk_renderer->destroy();
  m_ui_manager->destroy();
  m_gl_window->destroy();
  m_vk_window->destroy();
}
