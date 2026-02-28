set(CORE_PUBLIC_HEADERS
  include/rid.h
  include/resource_owner.h
  include/scene_view.h
  include/vertex.h
  include/material_params.h
  include/uniform_value.h
  include/uniform_set.h
  include/uniform_layout.h
  include/common.h
  include/vertex_layout.h
)

set(CORE_SOURCES
    src/dummy.cpp
    src/uniform_layout.cpp
)

add_library(core_module STATIC ${CORE_SOURCES} ${CORE_PUBLIC_HEADERS})

target_include_directories(core_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(core_module PRIVATE glm utils_module)
