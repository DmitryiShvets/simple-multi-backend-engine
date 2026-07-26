# Platform sources
set(PLATFORM_SOURCES
    desktop/desktop_platform.cpp
    desktop/glfw/glfw_main_window.cpp
    desktop/glfw/glfw_vk_gpu_context_creator.cpp
    desktop/glfw/glfw_gl_gpu_context_creator.cpp
    desktop/glfw/glfw_dx12_gpu_context_creator.cpp
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
