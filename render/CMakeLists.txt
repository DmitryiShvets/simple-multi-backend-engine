option(BUILD_OPENGL_RENDERER "Build OpenGL renderer" ON)
option(BUILD_VULKAN_RENDERER "Build Vulkan renderer" ON)

# (ОБЩАЯ ЧАСТЬ)
add_library(render_module STATIC
    src/render_factory.cpp
    src/render_scene.cpp
    src/render_graph.cpp
    src/render_pass.cpp
    src/render_graph_executor.cpp
    include/render_factory.h
    # RHI common headers
    include/render_device.h
    include/render_types.h
    include/resource_types.h
    include/uniform_factory_registry.h
    # Materials
    include/pipeline_config_registry.h
    src/pipeline_config_registry.cpp
    pipelines/default_pipeline.h
    pipelines/default_pipeline.cpp
    pipelines/ads_pipeline.h
    pipelines/ads_pipeline.cpp
    # Drawing Policy
    include/drawing_policy.h
    include/drawing_policy_registry.h
    src/drawing_policy_registry.cpp
    drawing_policies/default_drawing_policy.h
    drawing_policies/default_drawing_policy.cpp
    drawing_policies/ads_drawing_policy.h
    drawing_policies/ads_drawing_policy.cpp
)

target_include_directories(render_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/opengl/include>
)

target_link_libraries(render_module PRIVATE glm utils_module core_module imgui)

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
