set(WINDOW_PUBLIC_HEADERS
  include/glfw_main_window.h
  include/i_main_window.h
  include/window_factory.h
  include/imgui_impl_glfw.h
)

set(WINDOW_SOURCES
  src/glfw_main_window.cpp
  src/window_factory.cpp
  src/imgui_impl_glfw.cpp
)

add_library(platform_module STATIC ${WINDOW_SOURCES} ${WINDOW_PUBLIC_HEADERS})

target_include_directories(platform_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# DEPENDENCIES
target_link_libraries(platform_module PUBLIC glfw PRIVATE utils_module imgui )
