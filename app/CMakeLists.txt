#set(PROJ_NAME app)

set(APP_PUBLIC_HEADERS
  # Core
    include/application.h
)

set(APP_SOURCES
 main.cpp
  # Core
    application.cpp
)

add_executable (app ${APP_SOURCES} ${APP_PUBLIC_HEADERS})

target_include_directories(app
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(app PRIVATE adapters_module world_module render_module platform_module utils_module core_module glm ui_module imgui)
