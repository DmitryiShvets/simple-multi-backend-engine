# =============================================================================
# Find and configure external pre-built libraries
# Libraries are expected to be in build/libs/ directory
# =============================================================================

set(MY_EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/build/libs")

# ------------------------------------------------------------------------------
# GLFW
# ------------------------------------------------------------------------------
find_library(GLFW_LIB NAMES glfw3 glfw
    PATHS "${MY_EXTERNAL_DIR}/lib" "${MY_EXTERNAL_DIR}/lib64"
    NO_DEFAULT_PATH)
find_path(GLFW_INCLUDE NAMES GLFW/glfw3.h
    PATHS "${MY_EXTERNAL_DIR}/include"
    NO_DEFAULT_PATH)

if(NOT GLFW_LIB OR NOT GLFW_INCLUDE)
    message(FATAL_ERROR "GLFW not found! Run: ./scripts/build_libraries.sh")
endif()

add_library(external::glfw UNKNOWN IMPORTED)
set_target_properties(external::glfw PROPERTIES
    IMPORTED_LOCATION "${GLFW_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE}"
)

# ------------------------------------------------------------------------------
# GLM (header-only)
# ------------------------------------------------------------------------------
find_path(GLM_INCLUDE NAMES glm/glm.hpp
    PATHS "${MY_EXTERNAL_DIR}/include"
    NO_DEFAULT_PATH)

if(NOT GLM_INCLUDE)
    message(FATAL_ERROR "GLM not found! Run: ./scripts/build_libraries.sh")
endif()

add_library(external::glm INTERFACE IMPORTED)
set_target_properties(external::glm PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLM_INCLUDE}"
)

# ------------------------------------------------------------------------------
# Flecs
# ------------------------------------------------------------------------------
find_library(FLECS_LIB NAMES flecs
    PATHS "${MY_EXTERNAL_DIR}/lib" "${MY_EXTERNAL_DIR}/lib64"
    NO_DEFAULT_PATH)
find_path(FLECS_INCLUDE NAMES flecs.h
    PATHS "${MY_EXTERNAL_DIR}/include"
    NO_DEFAULT_PATH)

if(NOT FLECS_LIB OR NOT FLECS_INCLUDE)
    message(FATAL_ERROR "Flecs not found! Run: ./scripts/build_libraries.sh")
endif()

add_library(external::flecs UNKNOWN IMPORTED)
set_target_properties(external::flecs PROPERTIES
    IMPORTED_LOCATION "${FLECS_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${FLECS_INCLUDE}"
)

# ------------------------------------------------------------------------------
# ImGui
# ------------------------------------------------------------------------------
find_library(IMGUI_LIB NAMES imgui libimgui
    PATHS "${MY_EXTERNAL_DIR}/lib64" "${MY_EXTERNAL_DIR}/lib"
    NO_DEFAULT_PATH)

find_path(IMGUI_DIR NAMES imgui.h
    PATHS "${MY_EXTERNAL_DIR}/include/imgui"
    NO_DEFAULT_PATH)

get_filename_component(IMGUI_PARENT_DIR "${IMGUI_DIR}" DIRECTORY)

add_library(external::imgui STATIC IMPORTED GLOBAL)

set_target_properties(external::imgui PROPERTIES
    IMPORTED_LOCATION "${IMGUI_LIB}"
    IMPORTED_LOCATION_DEBUG "${IMGUI_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${IMGUI_DIR};${IMGUI_PARENT_DIR}"
)
