cmake_minimum_required(VERSION 3.12)
project(ExternalLibraries CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Настройки библиотек (разрешаем установку)
# ============================================================================
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL ON CACHE BOOL "" FORCE)  # <-- Важно для установки!

# ============================================================================
# Собираем библиотеки
# ============================================================================
add_subdirectory(flecs)
add_subdirectory(glfw)
add_subdirectory(glm)

# ============================================================================
# ImGui (с копированием CMakeLists.txt если нужно)
# ============================================================================
set(IMGUI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_CUSTOM_CMAKE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../patches/imgui.CMakeLists.txt")
set(IMGUI_CMAKE_DST "${IMGUI_SOURCE_DIR}/CMakeLists.txt")

if(NOT EXISTS "${IMGUI_CMAKE_DST}")
    file(COPY_FILE "${IMGUI_CUSTOM_CMAKE_SRC}" "${IMGUI_CMAKE_DST}")
endif()

add_subdirectory(imgui)

# ============================================================================
# Установка (для find_package и find_library)
# ============================================================================
include(GNUInstallDirs)

# GLFW уже имеет свою установку

# GLM - header only, устанавливаем вручную
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN "*.h"
    PATTERN "*.inl"
)

# ImGui - создаём установку вручную
install(TARGETS imgui
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Заголовки ImGui
install(DIRECTORY ${IMGUI_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imgui
    FILES_MATCHING PATTERN "*.h"
    PATTERN "backends/*"
    PATTERN "misc/*"
)
