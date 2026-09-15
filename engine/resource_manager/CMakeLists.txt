set(RESOURCES_SOURCES
    resources/buffer.cpp
    resources/pipeline.cpp
    resources/texture.cpp
    resources/shader.cpp
    resources/uniform_block.cpp
    resources/mesh.cpp
    resources/material.cpp
    resources/descriptor_set.cpp
)

set(LOADER_SOURCES
    loaders/shader_loader.cpp
    loaders/material_loader.cpp
    loaders/texture_loader.cpp
)

set(SOURCES
    resource_manager.cpp
    resource_handle_base.cpp
    slang_compiler.cpp
)

add_library(ssme_resources STATIC ${RESOURCES_SOURCES} ${LOADER_SOURCES} ${SOURCES})
add_library(ssme::resources ALIAS ssme_resources)

target_include_directories(ssme_resources
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

# DEPENDENCIES
target_link_libraries(ssme_resources
    PUBLIC
    PRIVATE
        ssme::rhi
        external::json
        external::slang
)
