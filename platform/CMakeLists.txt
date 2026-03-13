set(WINDOW_PUBLIC_HEADERS
  include/glfw_main_window.h
  include/i_main_window.h
  include/window_factory.h
  include/window_manager.h
)

set(WINDOW_SOURCES
  src/glfw_main_window.cpp
  src/window_factory.cpp
  src/window_manager.cpp
)

add_library(platform_module STATIC ${WINDOW_SOURCES} ${WINDOW_PUBLIC_HEADERS})

target_include_directories(platform_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# DEPENDENCIES
target_link_libraries(platform_module PRIVATE external::glfw utils_module external::imgui core_module)
