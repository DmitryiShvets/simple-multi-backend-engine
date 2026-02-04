#include "application.h"
#include "i_main_window.h"

// The application only needs the abstract renderer interface
#include "i_renderer.h"
#include "scene_view.h"

#include "ecs/components/render_component.h"
#include "ecs/components/transform_component.h"

#include <chrono>
#include <memory>

// The constructor now matches the new header, accepting two abstract renderers
Application::Application(std::unique_ptr<Window::IMainWindow> gl_window,
                         std::unique_ptr<Window::IMainWindow> vk_window,
                         std::unique_ptr<Render::IRenderer> gl_renderer,
                         std::unique_ptr<Render::IRenderer> vk_renderer)
    : m_gl_window(std::move(gl_window)), m_vk_window(std::move(vk_window)),
      m_gl_renderer(std::move(gl_renderer)),
      m_vulkan_renderer(std::move(vk_renderer)) {}

Application::~Application() = default;

void Application::init() {
  m_world = std::make_unique<Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>();
  m_render_system = std::make_unique<Core::Ecs::System::RenderSystem<
      Core::Ecs::World<Core::Ecs::FlecsWorldImpl>>>(*m_world);

  // Create test objects in the ECS
  auto circle = m_world->createEntity();
  m_world->addComponent<Core::Ecs::Component::Transform>(
      circle, glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f));
  m_world->addComponent<Core::Ecs::Component::Renderable>(
      circle, "circle", "custom", glm::vec3(1.0f, 0.0f, 0.0f));

  auto square = m_world->createEntity();
  m_world->addComponent<Core::Ecs::Component::Transform>(
      square, glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.2f));
  m_world->addComponent<Core::Ecs::Component::Renderable>(
      square, "quad", "custom", glm::vec3(1.0f, 0.0f, 1.0f));
}

void Application::run() {
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

    SceneView sceneView = m_render_system->createSceneView();

    // A single, abstract call to each renderer.
    // Each renderer is responsible for its own swapchain, commands, and
    // presentation.
    if (m_gl_renderer) {
      m_gl_renderer->renderFrame(sceneView);
    }
    if (m_vulkan_renderer) {
      m_vulkan_renderer->renderFrame(sceneView);
    }
    m_gl_window->swapBuffers();
    // m_vk_window->swapBuffers();
  }
}

void Application::close() {
  // In a real implementation, we would call a cleanup/shutdown method on each
  // renderer which would in turn call waitIdle(). if (m_vulkan_renderer)
  // m_vulkan_renderer->shutdown(); if (m_gl_renderer)
  // m_gl_renderer->shutdown();

  if (m_gl_window) {
    m_gl_window->destroy();
  }
  if (m_vk_window) {
    m_vk_window->destroy();
  }

}
