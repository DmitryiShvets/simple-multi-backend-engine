set(ECS_SOURCES
  ecs/entity.h
  dummy.cpp
)

add_library(module_world STATIC ${ECS_SOURCES})
add_library(ssme::world ALIAS module_world)

target_include_directories(module_world
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

# DEPENDENCIES
target_link_libraries(module_world
    PRIVATE
        ssme::common
)
