#pragma once

#include <cstddef>

namespace Core {

/**
 * @brief Тип рендер-бекенда
 * 
 * Используется для типобезопасного доступа к ресурсам разных бекендов
 */
enum class BackendType : size_t {
    OpenGL = 0,
    Vulkan = 1,
    Count  // Для проверки границ
};

} // namespace Core
