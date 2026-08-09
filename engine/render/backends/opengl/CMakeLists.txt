set(OPENGL_RENDER_SOURCES
    src/opengl_render_device.cpp
    src/opengl_shader_program.cpp
    src/opengl_shader_module.cpp
    src/opengl_buffer_objects.cpp
    src/opengl_command_list.cpp
    src/opengl_descriptor_set.cpp
    src/opengl_texture.cpp
)

add_library(ssme_opengl STATIC ${OPENGL_RENDER_SOURCES})
add_library(ssme::opengl ALIAS ssme_opengl)


target_include_directories(ssme_opengl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# DEPENDENCIES
find_package(OpenGL REQUIRED)
target_link_libraries(ssme_opengl
    PRIVATE
        ${OPENGL_LIBRARIES}
        external::glad
        external::glm
        external::imgui
        ssme::common
        ssme::rhi
        ssme::resources
)

# POST BUILD
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/res)
    add_custom_command(TARGET ssme_opengl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/res
            $<TARGET_FILE_DIR:ssme_opengl>/res
    )
endif()
