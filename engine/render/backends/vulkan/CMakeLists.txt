
set(VULKAN_RENDER_SOURCES
    src/vulkan_device.cpp
    src/vulkan_render_device.cpp
    src/vulkan_resource_manager.cpp
    src/vulkan_swap_chain.cpp
    src/vulkan_types.cpp
    src/vulkan_command_list.cpp
    src/vulkan_pipeline.cpp
    src/vulkan_pipeline_layout.cpp
    src/vulkan_descriptor_set.cpp
    src/vulkan_buffer.cpp
    src/vulkan_texture.cpp
    src/vulkan_helpers.cpp
)

add_library(ssme_vulkan STATIC ${VULKAN_RENDER_SOURCES})
add_library(ssme::vulkan ALIAS ssme_vulkan)

target_compile_definitions(ssme_vulkan PRIVATE
 VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
 VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

target_include_directories(ssme_vulkan
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# DEPENDENCIES
find_package(Vulkan REQUIRED)
target_link_libraries(ssme_vulkan
    PRIVATE
        Vulkan::Vulkan
        external::glm
        external::imgui
        ssme::common
        ssme::rhi
        ssme::platform
)


target_precompile_headers(ssme_vulkan PRIVATE
    <vulkan/vulkan_raii.hpp>
    <vulkan/vulkan.hpp>
    <vulkan/vulkan_core.h>
    <vulkan/vulkan.h>
    <vector>
    <memory>
    <cassert>
    <iostream>
)

# ======================================================================================
# AUTOMATIC SHADER COMPILATION (to build directory)
# ======================================================================================

# 1. Find the glslc compiler
find_program(GLSLC_EXECUTABLE glslc)
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Please install the Vulkan SDK and make sure glslc is in your PATH.")
endif()

# 2. Find all shader source files
file(GLOB SHADER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.glsl"
)

# 3. Create custom commands to compile shaders to build directory
set(SPIRV_OUTPUTS "")
foreach(SHADER_SOURCE IN ITEMS ${SHADER_SOURCES})
  get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
  # Output to build directory instead of source directory
  set(SPIRV_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/res/shaders/${SHADER_NAME}.spv")
  list(APPEND SPIRV_OUTPUTS ${SPIRV_OUTPUT})

  # Determine shader stage from filename prefix (v_ = vert, f_ = frag)
  if(SHADER_NAME MATCHES "^v_.*\\.glsl$")
    set(SHADER_STAGE "vert")
  elseif(SHADER_NAME MATCHES "^f_.*\\.glsl$")
    set(SHADER_STAGE "frag")
  elseif(SHADER_NAME MATCHES "^c_.*\\.glsl$")
    set(SHADER_STAGE "comp")
  elseif(SHADER_NAME MATCHES "^g_.*\\.glsl$")
    set(SHADER_STAGE "geom")
  elseif(SHADER_NAME MATCHES "^t_.*\\.glsl$")
    set(SHADER_STAGE "tesc")
  elseif(SHADER_NAME MATCHES "^e_.*\\.glsl$")
    set(SHADER_STAGE "tese")
  elseif(SHADER_NAME MATCHES "^r_.*\\.glsl$")
    set(SHADER_STAGE "rgen")
  elseif(SHADER_NAME MATCHES "^m_.*\\.glsl$")
    set(SHADER_STAGE "rmiss")
  elseif(SHADER_NAME MATCHES "^h_.*\\.glsl$")
    set(SHADER_STAGE "rchit")
  elseif(SHADER_NAME MATCHES "^i_.*\\.glsl$")
    set(SHADER_STAGE "rahit")
  elseif(SHADER_NAME MATCHES "^cc_.*\\.glsl$")
    set(SHADER_STAGE "rcall")
  elseif(SHADER_NAME MATCHES "^a_.*\\.glsl$")
    set(SHADER_STAGE "task")
  elseif(SHADER_NAME MATCHES "^ms_.*\\.glsl$")
    set(SHADER_STAGE "mesh")
  else()
    message(FATAL_ERROR "Unknown shader stage for file: ${SHADER_NAME}")
  endif()

  # Create output directory if it doesn't exist
  add_custom_command(
    OUTPUT  ${SPIRV_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/res/shaders"
    COMMAND ${GLSLC_EXECUTABLE} -fshader-stage=${SHADER_STAGE} -o ${SPIRV_OUTPUT} ${SHADER_SOURCE}
    DEPENDS ${SHADER_SOURCE}
    COMMENT "Compiling ${SHADER_NAME} to SPIR-V (${SHADER_STAGE})"
    VERBATIM)
endforeach()

# 4. Create a target that depends on all compiled shaders
add_custom_target(compile_shaders DEPENDS ${SPIRV_OUTPUTS})

# 5. Make the main library depend on the shader compilation target
add_dependencies(ssme_vulkan compile_shaders)

# 6. Copy shaders to bin directory (where the executable runs from)
# Copy each shader individually with proper dependency tracking
set(SHADER_BIN_OUTPUTS "")
foreach(SHADER_SOURCE IN ITEMS ${SHADER_SOURCES})
  get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
  set(SPIRV_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/res/shaders/${SHADER_NAME}.spv")
  set(SPIRV_BIN_OUTPUT "${CMAKE_SOURCE_DIR}/build/bin/res/shaders/${SHADER_NAME}.spv")
  list(APPEND SHADER_BIN_OUTPUTS ${SPIRV_BIN_OUTPUT})

  add_custom_command(
    OUTPUT  ${SPIRV_BIN_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SPIRV_OUTPUT} ${SPIRV_BIN_OUTPUT}
    DEPENDS ${SPIRV_OUTPUT}
    COMMENT "Copying ${SHADER_NAME}.spv to bin directory"
    VERBATIM)
endforeach()

add_custom_target(copy_shaders_to_bin ALL DEPENDS ${SHADER_BIN_OUTPUTS})
