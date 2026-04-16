# Platform sources
set(RHI_SOURCES
    dummy.cpp
)

# Create platform library
add_library(ssme_rhi STATIC ${RHI_SOURCES})
add_library(ssme::rhi ALIAS ssme_rhi)

# Include directories
target_include_directories(ssme_rhi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Dependencies
target_link_libraries(ssme_rhi
    PUBLIC
        ssme::common
    PRIVATE
        external::glfw
        external::imgui
        external::glad
)
