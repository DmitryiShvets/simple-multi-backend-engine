add_library(world_module INTERFACE)

add_subdirectory(ecs_core)
# add_subdirectory(scene)

target_link_libraries(world_module INTERFACE ecs_module)
