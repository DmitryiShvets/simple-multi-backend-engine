option(BUILD_OPENGL_RENDERER "Build OpenGL renderer" ON)
option(BUILD_VULKAN_RENDERER "Build Vulkan renderer" ON)

# (ОБЩАЯ ЧАСТЬ)
add_library(render_module STATIC
    src/render_factory.cpp
    src/render_scene.cpp
    src/render_graph_executor.cpp
    include/render_factory.h
    # RHI common headers
    include/render_device.h
    include/render_types.h
)

target_include_directories(render_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/opengl/include>
)

target_link_libraries(render_module PUBLIC glm utils_module core_module)

# OpenGL submodule
if(BUILD_OPENGL_RENDERER)
    add_subdirectory(opengl)
    target_link_libraries(render_module PRIVATE opengl_renderer)
endif()

# Vulkan submodule (dummy)
if(BUILD_VULKAN_RENDERER)
    add_subdirectory(vulkan)
    target_link_libraries(render_module PRIVATE vulkan_renderer)
endif()
