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
// Косяки
// 1) исправить хардкод в opengl_device.
// сейчас девайс должен знат про VertexN.
//  а что если он изменится. нужно доставать откуда-то размер вертекса:
//  uint64_t vertex_count = desc.size / sizeof(VertexN);
// 2) исправить хардкод в runtime_init_system.
// сейчас инит система должна знать какими данными инициализуется юниформа
// но в будущем она можетизмениться
//  нужно откуда то доставать конфиг юниформ
// Create material with uniform data
// Core::UniformSet mat_uniforms;
// mat_uniforms.set("color", Core::UniformValue(mat.color));
// 3) исправвить хардкод в runtime_init_system.
//  хорошо было бы доставать конфиг какую юниформу мы хотим создавать
//  Core::Uniforms::ObjectUniforms obj_uniforms{
//     .model_matrix = model_mat,
//     .normal_matrix = normal_mat
// };
// 4) Сейчас в runtime_update_system  nowledge Coupling
// `RuntimeInitSystem` знает внутреннюю структуру каждого материала (`mat.color`),
//  что нарушает **Dependency Rule**
// 4.а)  в файле runtime_update_system сразу создавать ObjectUniformsStd140
// миную промежуточные шаги
// 5) исправить хардкод в файле opengl_renderer
// сейчас рендерд должен знать какую пушконстанту нужно создавать
// и закидывать в draw_data.push_constants. нужно это решить
//     UniformValue model_matrix_val(per_object_data.model_matrix);
// model_matrix_val.setLabel("model_mat");
// draw_data.push_constants.emplace("model_mat", model_matrix_val);
// 6) еще есть проблема с map/unmap буфером в вулкане.
// нужно подмуть. возможно для HOST_COHERENT памяти
//  в Vulkan можно ержать буфер замапленным постоянно
// на протяжении всего времени жизни буфера
// 7) решить проблему с std140. как будто накладо всегда туда сбда гонять данные
// 8) решить проблему с frame resource. как это делать декларативно?
