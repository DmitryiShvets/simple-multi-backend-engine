cmake_minimum_required (VERSION 3.12)
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

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
# for linux fedora x11 required deps: sudo dnf install libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel
# for linux fedora wayland required deps: sudo dnf install extra-cmake-modules
add_subdirectory(external/flecs)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

# --- Configure and include imgui (cross-platform) ---
set(IMGUI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
set(IMGUI_CUSTOM_CMAKE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/patches/imgui.CMakeLists.txt")
set(IMGUI_CMAKE_DST "${IMGUI_SOURCE_DIR}/CMakeLists.txt")

# Check if the CMakeLists.txt exists, if not, copy our custom one.
# This makes the submodule usable without external tools like 'patch'.
if(NOT EXISTS "${IMGUI_CMAKE_DST}")
    message(STATUS "IMGUI: CMakeLists.txt not found. Copying custom version...")
    file(COPY_FILE "${IMGUI_CUSTOM_CMAKE_SRC}" "${IMGUI_CMAKE_DST}")
    message(STATUS "IMGUI: Custom CMakeLists.txt copied.")
else()
    message(STATUS "IMGUI: CMakeLists.txt already exists. Skipping copy.")
endif()

# Now, add imgui as a subdirectory
add_subdirectory(${IMGUI_SOURCE_DIR})
# --- End of imgui block ---

add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(adapters)
add_subdirectory(render)
add_subdirectory(platform)
add_subdirectory(ui)
add_subdirectory(world)
add_subdirectory(app)

# POST BUILD
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/build/compile_commands.json
)
