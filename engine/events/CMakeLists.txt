set(EVENT_SOURCES
  event_bus.cpp
)

add_library(ssme_event STATIC ${EVENT_SOURCES})
add_library(ssme::event ALIAS ssme_event)

target_include_directories(ssme_event
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

# DEPENDENCIES
target_link_libraries(ssme_event
    PRIVATE
        ssme::common
)
