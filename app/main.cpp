#include "application.h"
#include "glfw_vulkan_surface_creator.h"
#include "gpu_context_creator.h"
#include "render_factory.h"
#include "rendering_device_factory.h"
#include "window_factory.h"

int main() {
  auto vk_renderer = Render::RenderFactory::create(Render::API::Vulkan);
  auto gl_renderer = Render::RenderFactory::create(Render::API::OpenGL);

  Window::OpenGLGpuContextCreator gl_context_creator;
  Window::VulkanGpuContextCreator vk_context_creator;

  auto window_opengl = Window::WindowFactory::create(
      Window::API::GLFW, Window::WindowConfig{"Primitives", 800, 800},
      gl_context_creator);
  auto window_vulkan = Window::WindowFactory::create(
      Window::API::GLFW, Window::WindowConfig{"Primitives", 800, 800},
      vk_context_creator);

  // Получаем непрозрачный хэндл
  void *native_handle = window_vulkan->getNativeWindow();

  // Создаем конкретную стратегию
  Window::GlfwVulkanSurfaceCreator vk_surface_creator(native_handle);

  // Передаем стратегию в универсальную фабрику
  auto rendering_device =
      Render::RenderingDeviceFactory::create(vk_surface_creator);
  auto app = Application(std::move(window_opengl), std::move(gl_renderer),
                         std::move(rendering_device));
  app.init();
  app.run();
  app.close();
  return 0;
}
