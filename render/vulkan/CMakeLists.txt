set(VULKAN_RENDER_PUBLIC_HEADERS
    include/vulkan_renderer.h
    include/vulkan_device.h
    include/vulkan_rhi_device.h
    include/vulkan_resource_manager.h
    include/vulkan_swap_chain.h
    include/vulkan_types.h
    include/vulkan_command_list.h
    include/vulkan_pipeline.h
    include/vulkan_pipeline_layout.h
    include/vulkan_descriptor_set.h
    include/vulkan_buffer.h
    include/vulkan_texture.h
)

set(VULKAN_RENDER_SOURCES
    src/vulkan_renderer.cpp
    src/vulkan_device.cpp
    src/vulkan_rhi_device.cpp
    src/vulkan_resource_manager.cpp
    src/vulkan_swap_chain.cpp
    src/vulkan_types.cpp
    src/vulkan_command_list.cpp
    src/vulkan_pipeline.cpp
    src/vulkan_pipeline_layout.cpp
    src/vulkan_descriptor_set.cpp
    src/vulkan_buffer.cpp
    src/vulkan_texture.cpp
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
    PRIVATE Vulkan::Vulkan glm utils_module core_module render_module imgui
)

# ======================================================================================
# AUTOMATIC SHADER COMPILATION (in-place)
# ======================================================================================

# 1. Find the glslc compiler
find_program(GLSLC_EXECUTABLE glslc)
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Please install the Vulkan SDK and make sure glslc is in your PATH.")
endif()

# 2. Find all shader source files
file(GLOB SHADER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.frag"
)

# 3. Create custom commands to compile shaders in-place (in the source directory)
set(SPIRV_OUTPUTS "")
foreach(SHADER_SOURCE IN ITEMS ${SHADER_SOURCES})
  get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
  set(SPIRV_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/${SHADER_NAME}.spv")
  list(APPEND SPIRV_OUTPUTS ${SPIRV_OUTPUT})

  add_custom_command(
    OUTPUT  ${SPIRV_OUTPUT}
    COMMAND ${GLSLC_EXECUTABLE} -o ${SPIRV_OUTPUT} ${SHADER_SOURCE}
    DEPENDS ${SHADER_SOURCE}
    COMMENT "Compiling ${SHADER_NAME} to SPIR-V (in-place)"
    VERBATIM)
endforeach()

# 4. Create a target that depends on all compiled shaders
add_custom_target(compile_shaders DEPENDS ${SPIRV_OUTPUTS})

# 5. Make the main library depend on the shader compilation target
# This ensures shaders are compiled before the library is built.
add_dependencies(vulkan_renderer compile_shaders)


# ======================================================================================
# POST BUILD - Copy the entire 'res' directory (now containing .spv files)
# ======================================================================================
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/res)
    add_custom_command(TARGET vulkan_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/res
            $<TARGET_FILE_DIR:vulkan_renderer>/res
        COMMENT "Copying res directory to build output"
    )
endif()
