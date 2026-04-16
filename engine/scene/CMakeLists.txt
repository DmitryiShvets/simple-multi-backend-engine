set(SCENE_SOURCES
  scene.cpp
  render_proxy_builder.cpp
  scene_view_system.cpp
)

add_library(ssme_scene STATIC ${SCENE_SOURCES})
add_library(ssme::scene ALIAS ssme_scene)

target_include_directories(ssme_scene
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

# DEPENDENCIES
target_link_libraries(ssme_scene
    PRIVATE
        ssme::common
        ssme::world
        ssme::render
        ssme::resources
)
