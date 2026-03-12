set(OPENGL_RENDER_PUBLIC_HEADERS
    include/opengl_renderer.h
    include/opengl_device.h
    include/opengl_shader_program.h
    include/opengl_resource_manager.h
    include/opengl_buffer_objects.h
    include/opengl_command_list.h
    include/opengl_descriptor_set.h
)

set(OPENGL_RENDER_SOURCES
    src/opengl_renderer.cpp
    src/opengl_device.cpp
    src/opengl_shader_program.cpp
    src/opengl_resource_manager.cpp
    src/opengl_buffer_objects.cpp
    src/opengl_command_list.cpp
    src/opengl_descriptor_set.cpp
)

add_subdirectory(glad)

add_library(opengl_renderer STATIC ${OPENGL_RENDER_SOURCES} ${OPENGL_RENDER_PUBLIC_HEADERS})

target_include_directories(opengl_renderer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# DEPENDENCIES
find_package(OpenGL REQUIRED)
target_link_libraries(opengl_renderer
    PRIVATE ${OPENGL_LIBRARIES} glad external::glm utils_module core_module render_module external::imgui
)

# POST BUILD
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/res)
    add_custom_command(TARGET opengl_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/res
            $<TARGET_FILE_DIR:opengl_renderer>/res
    )
endif()
