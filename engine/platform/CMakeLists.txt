# Platform sources
set(PLATFORM_SOURCES
    desktop/desktop_platform.cpp
    desktop/backends/glfw/glfw_main_window.cpp
    desktop/backends/glfw/glfw_gpu_context_creator.cpp
)

# Create platform library
add_library(ssme_platform STATIC ${PLATFORM_SOURCES})
add_library(ssme::platform ALIAS ssme_platform)

# Include directories
target_include_directories(ssme_platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Dependencies
target_link_libraries(ssme_platform
    PUBLIC
        ssme::common
    PRIVATE
        external::glfw
        external::imgui
        external::glad
)
