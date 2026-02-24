#include "application.h"
#include "gpu_context_creator.h"

#include "window_factory.h"
#include "render_factory.h"
#include "ui_manager.h"
#include "glfw_vulkan_surface_creator.h"

#include <memory>

int main() {
  // --- Window Creation ---
  Window::OpenGLGpuContextCreator gl_gpu_ctx_creator;
  auto window_opengl = Window::WindowFactory::create(
      Window::API::GLFW, Window::WindowConfig{"OpenGL Window", 800, 600},
      gl_gpu_ctx_creator);

  Window::VulkanGpuContextCreator vk_gpu_ctx_creator;
  auto window_vulkan = Window::WindowFactory::create(
      Window::API::GLFW, Window::WindowConfig{"Vulkan Window", 800, 600},
      vk_gpu_ctx_creator);

  auto ui = std::make_unique<UI::UIManager>();
  ui->init(*window_vulkan, *window_opengl);

  // 1. Create the OpenGL Renderer using the simple overload
  auto gl_renderer = Render::RenderFactory::create(Render::API::OpenGL);
  gl_renderer->init(ui->getOpenGLContext());
  // 2. Create the Vulkan Renderer using the overload that takes a surface
  // creator
  void *native_handle = window_vulkan->getNativeWindow();
  Window::GlfwVulkanSurfaceCreator vk_surface_creator(native_handle);
  auto vk_renderer = Render::RenderFactory::create(vk_surface_creator);
  vk_renderer->init(ui->getVulkanContext());
  // --- Application Creation ---
  auto app = Application(std::move(window_opengl), std::move(window_vulkan),
                         std::move(gl_renderer), std::move(vk_renderer), std::move(ui));

  app.init();
  app.run();
  app.close();

  return 0;
}
