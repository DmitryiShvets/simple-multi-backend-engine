set(ECS_SYSTEMS_SOURCES
 "ecs/systems/scene_view_system.h"
)

set(ECS_COMPONENTS_SOURCES
    ecs/components/transform_component.h
 "ecs/components/runtime_component.h")

set(ECS_SOURCES
    "include/world.h"
    "src/blank.cpp"
 "include/ecs_types.h"
 "include/concepts.h"
    "include/query.h"
    "include/flecs_query.h"
    "include/flecs_world.h"
    include/objects_utils.h
    src/objects_utils.cpp
)

add_library(ecs_module STATIC
    ${ECS_SOURCES}
    ${ECS_COMPONENTS_SOURCES}
    ${ECS_SYSTEMS_SOURCES}
)

target_include_directories(ecs_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(ecs_module PUBLIC external::flecs PRIVATE external::glm core_module utils_module)
