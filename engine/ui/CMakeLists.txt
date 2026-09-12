set(UI_SOURCES
    hello_widget.cpp
    imgui_backend.cpp
    ui_manager.cpp
)

add_library(ssme_ui STATIC ${UI_SOURCES})
add_library(ssme::ui ALIAS ssme_ui)

target_include_directories(ssme_ui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

# DEPENDENCIES
target_link_libraries(ssme_ui
    PRIVATE
        external::imgui
        external::glfw
        ssme::platform
        ssme::event
)
