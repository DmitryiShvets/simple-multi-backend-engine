set(APP_SOURCES
 main.cpp
)

add_executable (app ${APP_SOURCES})

target_include_directories(app
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(app PRIVATE ssme::engine)
