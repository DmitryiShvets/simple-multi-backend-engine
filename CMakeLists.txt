cmake_minimum_required(VERSION 3.12)
set(PROJECT_NAME PRIMITEVES)
project(${PROJECT_NAME})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
  # Use the PDB server for concurrent access during parallel builds
  add_compile_options("/FS")
  add_compile_options(/Z7)
endif()

# EXTERNAL LIBS (Pre-built IN build/libs)
include(external/FindExternalLibs.cmake)

# INNER LIBS
add_subdirectory(engine)
add_subdirectory(app_new)


# POST BUILD
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/build/compile_commands.json
)
