set(VULKAN_RENDER_PUBLIC_HEADERS
    include/vulkan_renderer.h
    include/vulkan_device.h
    include/vulkan_rendering_device.h
    include/vulkan_resource_manager.h
    include/vulkan_swap_chain.h
    include/vulkan_types.h
    include/vulkan_command_list.h
    include/vulkan_pipeline.h
    include/simple_render_system.h
    include/vulkan_descriptor_set.h
)

set(VULKAN_RENDER_SOURCES
    src/vulkan_renderer.cpp
    src/vulkan_device.cpp
    src/vulkan_rendering_device.cpp
    src/vulkan_resource_manager.cpp
    src/vulkan_swap_chain.cpp
    src/vulkan_types.cpp
    src/vulkan_command_list.cpp
    src/vulkan_pipeline.cpp
    src/simple_render_system.cpp
    src/vulkan_descriptor_set.cpp
)

add_library(vulkan_renderer STATIC ${VULKAN_RENDER_SOURCES} ${VULKAN_RENDER_PUBLIC_HEADERS})

target_include_directories(vulkan_renderer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/render/include>
)

# DEPENDENCIES
find_package(Vulkan REQUIRED)
target_link_libraries(vulkan_renderer
    PUBLIC Vulkan::Vulkan
    PRIVATE glm utils_module
)

# POST BUILD
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/res)
    add_custom_command(TARGET vulkan_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/res
            $<TARGET_FILE_DIR:vulkan_renderer>/res
    )
endif()
