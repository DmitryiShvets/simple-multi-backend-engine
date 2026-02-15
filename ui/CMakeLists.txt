set(UI_PUBLIC_HEADERS
    include/hello_widget.h
)
set(UI_SOURCES
    src/hello_widget.cpp
)

add_library(ui_module STATIC ${UI_SOURCES} ${UI_PUBLIC_HEADERS})

target_include_directories(ui_module PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# DEPENDENCIES
target_link_libraries(ui_module PRIVATE imgui)
