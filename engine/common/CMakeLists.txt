set(CORE_SOURCES
  core/uniform_layout.cpp
)

set(UTILS_SOURCES
    utils/logger.cpp
    utils/common_utils.cpp
    utils/curve_utils.cpp
    utils/image_loader.cpp
    utils/sphere.cpp
)

add_library(ssme_common STATIC ${CORE_SOURCES} ${UTILS_SOURCES})
add_library(ssme::common ALIAS ssme_common)

target_include_directories(ssme_common
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

# DEPENDENCIES
target_link_libraries(ssme_common PUBLIC external::glm)
