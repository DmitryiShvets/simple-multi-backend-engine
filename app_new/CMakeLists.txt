set(APP_SOURCES
 main.cpp
)

add_executable (app ${APP_SOURCES})

target_include_directories(app
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(app PRIVATE ssme::engine)

add_custom_command(TARGET app POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/build/libs/bin/flecs.dll"
    "$<TARGET_FILE_DIR:app>"
  COMMENT "Copying flecs.dll to output directory"
)
