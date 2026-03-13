#include "application.h"
#include "backend_type.h" // ← Core::BackendType
#include "render_manager.h"
#include "window_manager.h"

#include "common_utils.h"
#include "objects_utils.h"
#include "scene_view.h"

#include "ecs/components/material_component.h"
#include "ecs/components/transform_component.h"
#include "ecs/systems/scene_view_system.h"
#include "flecs_world.h"
#include "world.h"

#include "runtime/runtime_init_system.h"
#include "runtime/runtime_update_system.h"

#include "hello_widget.h"
#include "ui_manager.h"
#include "uniform_factory_registry.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

// Forward declare ImGui types
struct ImDrawData;

Application::Application(std::unique_ptr<Window::WindowManager> window_manager,
                         std::unique_ptr<Render::RenderManager> render_manager,
                         std::unique_ptr<UI::UIManager> ui_manager)
    : m_window_manager(std::move(window_manager)),
      m_render_manager(std::move(render_manager)),
      m_ui_manager(std::move(ui_manager)) {}

Application::~Application() = default;

void Application::init() {
  // Set window positions
  m_window_manager->setWindowPosition(Core::BackendType::OpenGL, {100, 100});
  m_window_manager->setWindowPosition(Core::BackendType::Vulkan, {950, 100});
  m_world = std::make_unique<Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>();

  // Create test objects in the ECS

  // Triangle with Vertex (position only) - uses default material
  auto triangle1 = createTriangle(*m_world, "triangle1",
                                  glm::vec3(0.0f, 1.0f, 0.0f), "default");
  m_world->addComponent<Core::Ecs::Component::DefaultMaterial>(
      triangle1, Core::Ecs::Component::DefaultMaterial{
                     .color = glm::vec3(0.5f, 0.5f, 1.0f)});

  // Triangle with VertexN (position + normal) - uses ads material
  auto triangle2 = createTriangleN(*m_world, "triangle2",
                                   glm::vec3(0.0f, -1.0f, 0.0f), "ads");
  m_world->addComponent<Core::Ecs::Component::AdsMaterial>(
      triangle2,
      Core::Ecs::Component::AdsMaterial{.color = glm::vec3(0.0f, 1.0f, 0.0f)});

  // Sphere with VertexN - uses ads material
  auto sphere1 = createSphere(*m_world, "sphere1", 0.5f, 32, 32, "ads");
  m_world->addComponent<Core::Ecs::Component::AdsMaterial>(
      sphere1,
      Core::Ecs::Component::AdsMaterial{.color = glm::vec3(0.0f, 0.0f, 1.0f)});

  // auto sphere2 = createSphere(*m_world, "sphere2", 0.6f, 32, 32, "ads");
  // m_world->addComponent<Core::Ecs::Component::AdsMaterial>(
  //     sphere2,
  //     Core::Ecs::Component::AdsMaterial{.color = glm::vec3(1.0f, 1.0f,
  //     0.0f)});

  // --- Register uniform factories ---
  // This must be called before RuntimeInitSystem is created
  Render::registerUniformFactories();

  // --- Initialize runtime resources ---
  auto runtime_init_system =
      std::make_unique<Core::Ecs::System::RuntimeInitSystem<
          Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>>(*m_world,
                                                        *m_render_manager);
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
          Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>>(*m_world,
                                                        *m_render_manager);

  auto lastTime = std::chrono::high_resolution_clock::now();

  UI::HelloWidget widget;
  // The main loop is now extremely simple and clean.
  while (m_window_manager->allAlive()) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime =
        std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    m_window_manager->update();

    m_ui_manager->render([&widget]() { widget.render(); });
    // test only
    auto triangle = m_world->getEntity("triangle2");
    auto &transform =
        m_world->getComponent<Core::Ecs::Component::Transform>(triangle);
    transform.position.y = CUtils::lerp(-1, 1, widget.m_slider_value);
    transform.position.x = CUtils::lerp(-1, 1, widget.m_slider_value);
    auto sphere1 = m_world->getEntity("sphere1");
    auto &transform1 =
        m_world->getComponent<Core::Ecs::Component::Transform>(sphere1);
    transform1.position.x = CUtils::lerp(-1, 1, widget.m_slider_value);
    // transform1.position.z = -1;
    // auto sphere2 = m_world->getEntity("sphere2");
    // auto &transform2 =
    //     m_world->getComponent<Core::Ecs::Component::Transform>(sphere2);
    // transform2.position.x = widget.m_slider_value;
    // transform2.position.z = 1;
    // Update per-object uniform buffers before rendering
    runtime_update_system->update();

    Core::SceneView gl_scene_view = gl_scene_view_system->run();
    Core::SceneView vk_scene_view = vk_scene_view_system->run();

    // Use BackendType for type-safe access
    std::vector<Core::SceneView> scenes = {gl_scene_view, vk_scene_view};
    std::vector<ImDrawData *> ui_draw_bundle =  m_ui_manager->getBundleDrawData();
    m_render_manager->frame(scenes, ui_draw_bundle);

    m_window_manager->swapBuffers();
  }
}

void Application::close() {
  m_render_manager->destroy();
  m_ui_manager->destroy();
  m_window_manager->destroy();
}
