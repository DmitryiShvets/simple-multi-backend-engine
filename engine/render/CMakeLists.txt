option(BUILD_OPENGL_RENDERER "Build OpenGL renderer" ON)
option(BUILD_VULKAN_RENDERER "Build Vulkan renderer" ON)
if(WIN32)
    option(BUILD_DX12_RENDERER "Build DirectX 12 renderer" ON)
else()
    set(BUILD_DX12_RENDERER OFF)
endif()

set(RENDER_SOURCES
    graph/render_graph.cpp
    graph/render_pass.cpp
    graph/transient_pool.cpp

    renderer.cpp
    opengl_renderer.cpp
    vulkan_renderer.cpp
    render_system.cpp
    camera_controller.cpp
    camera.cpp
)
# d3d12_renderer.cpp only on Windows
if(WIN32)
list(APPEND RENDER_SOURCES d3d12_renderer.cpp)
endif()

add_library(ssme_render STATIC ${RENDER_SOURCES})
add_library(ssme::render ALIAS ssme_render)

target_include_directories(ssme_render
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

target_link_libraries(ssme_render
    PUBLIC
        ssme::rhi
    PRIVATE
        external::glm
        external::imgui
        ssme::common
        ssme::resources
        ssme::input
)

# OpenGL submodule
if(BUILD_OPENGL_RENDERER)
    target_link_libraries(ssme_render PRIVATE ssme::opengl)
endif()

# Vulkan submodule
if(BUILD_VULKAN_RENDERER)
    target_link_libraries(ssme_render PRIVATE ssme::vulkan)
endif()

# DirectX 12 submodule
if(BUILD_DX12_RENDERER)
    target_link_libraries(ssme_render PRIVATE ssme::dx12)
endif()
