#pragma once

#include <iostream>
#include <source_location>
#include <string_view>
#include <format> // C++20 feature for convenient text formatting

namespace ssme {

// Enable checks only in Debug builds
#ifdef NDEBUG
    constexpr bool DEBUG_MODE = false;
#else
    constexpr bool DEBUG_MODE = true;
#endif

/**
 * @brief Modern assert for the engine
 */
inline void debug_assert(bool condition,
                          std::string_view message = "",
                          const std::source_location loc = std::source_location::current()) {
    if constexpr (DEBUG_MODE) {
        if (!condition) {
            // Form a nice report
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

            // 1. Write to your logger
            // Logger::error_log(error_msg);

            // 2. Duplicate to console for instant reaction
            std::cerr << error_msg << std::endl;

            // 3. Stop execution (Breakpoint)
            #if defined(_MSC_VER)
                __debugbreak();
            #else
                __builtin_trap();
            #endif
        }
    }
}

} // namespace ssme
