set(HELPER_PUBLIC_HEADERS
  include/glfw_vulkan_surface_creator.h
  include/gpu_context_creator.h
  include/ui_manager.h
  include/imgui_backend.h
)

set(HELPER_SOURCES
    src/glfw_vulkan_surface_creator.cpp
    src/gpu_context_creator.cpp
    src/ui_manager.cpp
    src/imgui_backend.cpp
)

add_library(adapters_module STATIC ${HELPER_SOURCES} ${HELPER_PUBLIC_HEADERS})

target_include_directories(adapters_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# DEPENDENCIES
target_link_libraries(adapters_module PRIVATE external::imgui glad platform_module utils_module render_module ui_module core_module)
