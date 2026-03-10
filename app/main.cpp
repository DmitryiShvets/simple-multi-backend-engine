#include "application.h"
#include "gpu_context_creator.h"

#include "glfw_vulkan_surface_creator.h"
#include "logger.h"
#include "render_factory.h"
#include "ui_manager.h"
#include "window_factory.h"

#include <cstdlib>
#include <exception>
#include <memory>
#include <stdexcept>

int main() {

  try {
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
                           std::move(gl_renderer), std::move(vk_renderer),
                           std::move(ui));

    app.init();
    app.run();
    app.close();
  } catch (const std::exception &e) {
    Logger::error_log(e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
// ============================================================================
// Технические долги и проблемы архитектуры
// ============================================================================
//
// ✅ Исправлен. Пункт 1 (хардкод созданя вершинных буферов в
// RuntimeInitSystem):
//    - Вершинные буферы при создании берут данные из VertexLayout
//    - Каждый материал использует свой тип вершин.
//    - При создании пайплайн лайуаутов createMatreial() использует VertexLayout
//    - RuntimeInitSystem использует VertexLayout
//
// ✅ Исправлен. Пункт 2 (хардкод созданя юнифррм в RuntimeInitSystem):
//    - Фабрики конвертируют material data → UniformSet
//    - RuntimeInitSystem использует Render::UniformFactoryRegistry через DI
//
// -----------------------------------------------------------------------------
// ТРЕБУЕТ ИСПРАВЛЕНИЯ:
// -----------------------------------------------------------------------------
//
// 🔴 Пункт 3+4: Knowledge Coupling в RuntimeUpdateSystem
//    Проблема: Система знает внутреннюю структуру ObjectUniforms
//    Файлы: app/runtime/runtime_update_system.h:41-43,
//           app/runtime/runtime_init_system.h:191-194
// ⚪ Пункт 4а: Лишняя конвертация ObjectUniforms → ObjectUniformsStd140
//    Проблема: Каждый кадр делается двойная конвертация
//    Файлы: app/runtime/runtime_update_system.h:41-43
//    Решение: Использовать ObjectUniformsStd140 напрямую, убрать промежуточный
//    слой
// 🔴 Пункт 5: Хардкод push-констант в renderer
//    Проблема: OpenGLRenderer знает имя "model_mat" и создаёт UniformValue
//    Файлы: render/opengl/src/opengl_renderer.cpp:122-124,
//           render/vulkan/src/vulkan_renderer.cpp (аналогично)
//    Решение: Переместить в DrawingPolicy (политика знает, какие константы
//    нужны)
// ⚪ Пункт 6: map/unmap буферов в Vulkan
//    Проблема: Неоптимальная работа с памятью (map/unmap каждый кадр)
//    Файлы: render/vulkan/src/vulkan_rhi_device.cpp
//    Решение: Для HOST_COHERENT памяти держать буфер замапленным постоянно
// 🔴 Пункт 7: Конвертация std140 каждый кадр
//    Проблема: Данные конвертируются между ObjectUniforms и
//    ObjectUniformsStd140 Файлы: app/runtime/runtime_update_system.h:41-43
// 🔴 Пункт 8: Declarative per-frame resources
//    Проблема: Per-frame ресурсы создаются императивно в
//    createPerFrameResources() Файлы:
//    render/opengl/src/opengl_renderer.cpp:140-170,
//           render/vulkan/src/vulkan_renderer.cpp:247-280
//    Решение: Использовать декларативный конфиг (аналогично pipeline config)
// ============================================================================
// TODO: Список будущих улучшений
// ============================================================================
// 1. Создать raii обвертку над imgui
// 2. Придумать и реализовать новую систему управления ресурсами
