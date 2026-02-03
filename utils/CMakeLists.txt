set(UTILS_PUBLIC_HEADERS
    include/logger.h
    include/common_utils.h
    include/curve_utils.h
    include/vector2.h
    include/segment.h
    include/polyline.h
)

set(UTILS_SOURCES
    src/logger.cpp
    src/common_utils.cpp
    src/curve_utils.cpp
)

add_library(utils_module STATIC ${UTILS_SOURCES} ${UTILS_PUBLIC_HEADERS})

target_include_directories(utils_module
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(utils_module PRIVATE glm)
