set(DX12_RENDER_SOURCES
 src/dx12_command_list.cpp
 src/dx12_render_device.cpp
 src/dx12_device.cpp
 src/dx12_swap_chain.cpp
 src/dx12_helpers.cpp
 src/dx12_buffer.cpp
 src/dx12_texture.cpp
 src/dx12_shader_module.cpp
 src/dx12_pipeline_layout.cpp
 src/dx12_pipeline.cpp
 src/dx12_descriptor_set.cpp
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
        dxguid
        d3dcompiler
        external::imgui
        ssme::common
        ssme::rhi
        ssme::platform
        ssme::resources
)

# ======================================================================================
# AUTOMATIC DX12 SHADER COMPILATION (.hlsl → .cso)
# ======================================================================================

# 1. Find the dxc compiler
find_program(DXC_EXECUTABLE dxc
  PATHS
    "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64/"
    "$ENV{VULKAN_SDK}/bin/"
)
if(NOT DXC_EXECUTABLE)
  message(FATAL_ERROR "dxc not found. Install Windows SDK (10.0.20348+) or Vulkan SDK.")
endif()

# 2. Find all HLSL shader source files
file(GLOB HLSL_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.hlsl"
)

# 3. Create custom commands to compile shaders to build directory
set(CSO_OUTPUTS "")
foreach(HLSL IN ITEMS ${HLSL_SHADERS})
  get_filename_component(SHADER_NAME ${HLSL} NAME)
  set(CSO_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/res/shaders/${SHADER_NAME}.cso")
  list(APPEND CSO_OUTPUTS ${CSO_OUTPUT})

  # Determine shader profile & entry point from filename prefix (v_ = vertex, f_ = pixel)
  if(SHADER_NAME MATCHES "^v_.*\\.hlsl$")
    set(SHADER_PROFILE "vs_6_0")
    set(ENTRY_POINT "VSMain")
  elseif(SHADER_NAME MATCHES "^f_.*\\.hlsl$")
    set(SHADER_PROFILE "ps_6_0")
    set(ENTRY_POINT "PSMain")
  else()
    message(FATAL_ERROR "Unknown shader stage for file: ${SHADER_NAME}")
  endif()
  set(DEP_FILE "${CSO_OUTPUT}.d")

  # Create output directory if it doesn't exist
  add_custom_command(
    OUTPUT  ${CSO_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/res/shaders"
    COMMAND ${DXC_EXECUTABLE} -T ${SHADER_PROFILE} -E ${ENTRY_POINT}
            -Fo "${CSO_OUTPUT}" ${HLSL}
    DEPENDS ${HLSL}
    COMMENT "Compiling ${SHADER_NAME} to DXIL (${SHADER_PROFILE})"
    VERBATIM)
endforeach()

# 4. Create a target that depends on all compiled shaders
add_custom_target(compile_dx12_shaders DEPENDS ${CSO_OUTPUTS})

# 5. Make the main library depend on the shader compilation target
add_dependencies(ssme_dx12 compile_dx12_shaders)

# 6. Copy shaders to bin directory (where the executable runs from)
set(CSO_BIN_OUTPUTS "")
foreach(HLSL IN ITEMS ${HLSL_SHADERS})
  get_filename_component(SHADER_NAME ${HLSL} NAME)
  set(CSO_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/res/shaders/${SHADER_NAME}.cso")
  set(CSO_BIN_OUTPUT "${CMAKE_SOURCE_DIR}/build/bin/res/shaders/${SHADER_NAME}.cso")
  list(APPEND CSO_BIN_OUTPUTS ${CSO_BIN_OUTPUT})

  add_custom_command(
    OUTPUT  ${CSO_BIN_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CSO_OUTPUT} ${CSO_BIN_OUTPUT}
    DEPENDS ${CSO_OUTPUT}
    COMMENT "Copying ${SHADER_NAME}.cso to bin directory"
    VERBATIM)
endforeach()

add_custom_target(copy_dx12_shaders_to_bin ALL DEPENDS ${CSO_BIN_OUTPUTS})

add_custom_target(force_recompile_dx12_shaders
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning compiled DX12 shaders..."
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/res/shaders"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/res/shaders"
    COMMENT "DX12 shader cache cleared. Run build again to recompile."
)
