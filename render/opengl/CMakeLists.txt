set(OPENGL_RENDER_PUBLIC_HEADERS
    include/opengl_renderer.h
    include/opengl_device.h
    include/opengl_shader_program.h
    include/opengl_resource_manager.h
    include/opengl_buffer_objects.h
)

set(OPENGL_RENDER_SOURCES
    src/opengl_renderer.cpp
    src/opengl_device.cpp
    src/opengl_shader_program.cpp
    src/opengl_resource_manager.cpp
    src/opengl_buffer_objects.cpp
)

add_library(opengl_renderer STATIC ${OPENGL_RENDER_SOURCES} ${OPENGL_RENDER_PUBLIC_HEADERS})

target_include_directories(opengl_renderer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/render/include>
)

# DEPENDENCIES
find_package(OpenGL REQUIRED)
target_link_libraries(opengl_renderer
    PRIVATE ${OPENGL_LIBRARIES} glm utils_module core_module
    PUBLIC glad
)

# POST BUILD
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/res)
    add_custom_command(TARGET opengl_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/res
            $<TARGET_FILE_DIR:opengl_renderer>/res
    )
endif()
