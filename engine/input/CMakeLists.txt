set(INPUT_SOURCES
  input_system.cpp
)

add_library(ssme_input STATIC ${INPUT_SOURCES})
add_library(ssme::input ALIAS ssme_input)

target_include_directories(ssme_input
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

target_link_libraries(ssme_input
    PUBLIC
        ssme::event
    PRIVATE
        ssme::common
)
