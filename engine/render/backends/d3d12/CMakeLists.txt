set(DX12_RENDER_SOURCES
 src/dx12_command_list.cpp
 src/dx12_render_device.cpp
 src/dx12_device.cpp
 src/dx12_swap_chain.cpp
 src/dx12_helpers.cpp
 src/dx12_buffer.cpp
 src/dx12_texture.cpp
)

add_library(ssme_dx12 STATIC ${DX12_RENDER_SOURCES})
add_library(ssme::dx12 ALIAS ssme_dx12)


target_include_directories(ssme_dx12
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# DEPENDENCIES
target_link_libraries(ssme_dx12
    PRIVATE
        d3d12
        dxgi
        d3dcompiler
        external::imgui
        ssme::common
        ssme::rhi
        ssme::platform
        ssme::resources
)

# POST BUILD
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/res)
    add_custom_command(TARGET ssme_dx12 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/res
            $<TARGET_FILE_DIR:ssme_dx12>/res
    )
endif()
