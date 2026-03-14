# Gladd sources
set(GLAD_SOURCES
    src/gl.c
    include/glad/gl.h
)

# Create glad library
add_library(glad STATIC ${GLAD_SOURCES})
add_library(external::glad ALIAS glad)

# Include directories
target_include_directories(glad
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
