#include "application.h"
#include "gpu_context_creator.h"

#include "window_factory.h"
#include "render_factory.h"

#include "glfw_vulkan_surface_creator.h"

#include <memory>

int main() {
  // --- Window Creation ---
  Window::OpenGLGpuContextCreator gl_context_creator;
  auto window_opengl = Window::WindowFactory::create(
      Window::API::GLFW, Window::WindowConfig{"OpenGL Window", 800, 600},
      gl_context_creator);

  Window::VulkanGpuContextCreator vk_context_creator;
  auto window_vulkan = Window::WindowFactory::create(
      Window::API::GLFW, Window::WindowConfig{"Vulkan Window", 800, 600},
      vk_context_creator);

  // --- Renderer Creation using the unified factory ---

  // 1. Create the OpenGL Renderer using the simple overload
  auto gl_renderer = Render::RenderFactory::create(Render::API::OpenGL);

  // 2. Create the Vulkan Renderer using the overload that takes a surface
  // creator
  void *native_handle = window_vulkan->getNativeWindow();
  Window::GlfwVulkanSurfaceCreator vk_surface_creator(native_handle);
  auto vk_renderer = Render::RenderFactory::create(vk_surface_creator);

  // --- Application Creation ---
  auto app = Application(std::move(window_opengl), std::move(window_vulkan),
                         std::move(gl_renderer), std::move(vk_renderer));

  app.init();
  app.run();
  app.close();

  return 0;
}
