add_library(core_module INTERFACE)

add_subdirectory(ecs_core)
# add_subdirectory(scene)

target_link_libraries(core_module INTERFACE ecs_module)
