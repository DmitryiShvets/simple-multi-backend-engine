set(WINDOW_PUBLIC_HEADERS
  include/glfw_main_window.h
  include/i_main_window.h
  include/window_factory.h
)

set(WINDOW_SOURCES
  src/glfw_main_window.cpp
  src/window_factory.cpp
)

add_library(window_module STATIC ${WINDOW_SOURCES} ${WINDOW_PUBLIC_HEADERS})

target_include_directories(window_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# DEPENDENCIES
target_link_libraries(window_module PUBLIC glfw PRIVATE utils_module)
