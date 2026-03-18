/**
 * @brief New application entry point using ssme engine
 *
 * This is the new application that uses the ssme engine
 * with proper abstraction layers.
 */

#include "engine.h"
#include <cstdlib>
#include <iostream>

// Constants
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
int main() {

  try {
    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);
    engine.run();
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
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
