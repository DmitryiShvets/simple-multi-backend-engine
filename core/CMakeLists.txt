set(CORE_PUBLIC_HEADERS
  include/rid.h
  include/resource_owner.h
  include/scene_view.h
)

set(CORE_SOURCES
    src/dummy.cpp
)

add_library(core_module STATIC ${CORE_SOURCES} ${CORE_PUBLIC_HEADERS})

target_include_directories(core_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
