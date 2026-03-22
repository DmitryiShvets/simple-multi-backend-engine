#pragma once

#include <iostream>
#include <source_location>
#include <string_view>
#include <format> // C++20 фича для удобного текста

namespace ssme {

// Включаем проверки только в Debug-сборке
#ifdef NDEBUG
    constexpr bool DEBUG_MODE = false;
#else
    constexpr bool DEBUG_MODE = true;
#endif

/**
 * @brief Современный ассерт для движка
 */
inline void debug_assert(bool condition,
                          std::string_view message = "",
                          const std::source_location loc = std::source_location::current()) {
    if constexpr (DEBUG_MODE) {
        if (!condition) {
            // Формируем красивый отчет
            std::string error_msg = std::format(
                "\n--- ASSERTION FAILED ---\n"
                "Message:  {}\n"
                "Function: {}\n"
                "File:     {}:{}:{}\n"
                "-------------------------",
                message,
                loc.function_name(),
                loc.file_name(),
                loc.line(),
                loc.column()
            );

            // 1. Пишем в твой логгер
            // Logger::error_log(error_msg);

            // 2. Дублируем в консоль для мгновенной реакции
            std::cerr << error_msg << std::endl;

            // 3. Останавливаем выполнение (Breakpoint)
            #if defined(_MSC_VER)
                __debugbreak();
            #else
                __builtin_trap();
            #endif
        }
    }
}

} // namespace ssme
