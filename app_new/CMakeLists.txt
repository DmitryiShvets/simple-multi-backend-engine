set(APP_SOURCES
 main.cpp
)

add_executable (app_new ${APP_SOURCES})

target_include_directories(app_new
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(app_new PRIVATE ssme::engine)
