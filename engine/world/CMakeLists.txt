set(ECS_SOURCES
  ecs/entity.h
  dummy.cpp
)

add_library(ssme_world STATIC ${ECS_SOURCES})
add_library(ssme::world ALIAS ssme_world)

target_include_directories(ssme_world
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

# DEPENDENCIES
target_link_libraries(ssme_world
    PRIVATE
        ssme::common
)
