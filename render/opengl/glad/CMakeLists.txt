cmake_minimum_required(VERSION 3.12)
project(glad)

set(GLAD_SRC src/gl.c include/glad/gl.h)

add_library(glad STATIC ${GLAD_SRC})

target_include_directories(glad PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
