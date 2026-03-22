set(RESOURCES_SOURCES
    resources/buffer.cpp
)

set(SOURCES
    resource_manager.cpp
    resource_handle_base.cpp
)

add_library(ssme_resources STATIC ${RESOURCES_SOURCES} ${SOURCES})
add_library(ssme::resources ALIAS ssme_resources)

target_include_directories(ssme_resources
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

# DEPENDENCIES
target_link_libraries(ssme_resources
    PRIVATE
        ssme::rhi
)
