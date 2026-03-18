cmake_minimum_required(VERSION 3.12)

project(SSME VERSION 1.0.0 LANGUAGES CXX C)

# Internal libs
add_subdirectory(common)
add_subdirectory(platform)
add_subdirectory(ui)
add_subdirectory(render/rhi)
add_subdirectory(render/backends/opengl)
add_subdirectory(render/backends/vulkan)
add_subdirectory(render)
add_subdirectory(world)

# Engine sources
set(ENGINE_SOURCES
  engine.cpp
)

# Create main target
add_library(ssme_engine STATIC ${ENGINE_SOURCES})
add_library(ssme::engine ALIAS ssme_engine)

target_include_directories(ssme_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

# DEPENDENCIES
target_link_libraries(ssme_engine
    PUBLIC
        ssme::common
    PRIVATE
        ssme::rhi
        ssme::platform
        ssme::ui
        ssme::render
        ssme::world
)
