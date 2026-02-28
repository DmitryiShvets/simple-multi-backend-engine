#pragma once

#include "glm/ext/vector_float3.hpp"
#include "uniform_value.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_access.hpp>  // for glm::column()

namespace Core::Uniforms {

// ============================================================================
// Per-Frame Uniforms (Descriptor Set 0)
// Updated once per frame, shared across all objects
// ============================================================================
struct FrameUniforms {
  glm::mat4 view_projection = glm::mat4(1.0f);
  glm::vec3 light_position =  glm::vec3(0.0f);
  glm::vec3 Kd = glm::vec3(0.0f);
  glm::vec3 Ld = glm::vec3(0.0f);
  glm::vec3 camera_position = glm::vec3(0.0f);
};

// std140-compatible struct for GPU buffer (112 bytes total)
// glm::mat3 in std140 is stored as 3 vec4 columns (each column aligned to 16 bytes)
struct alignas(16) FrameUniformsStd140 {
    glm::mat4 view_projection;      // 64 bytes
    glm::vec3 light_position;       // 12 bytes
    float _pad0;                 // 4 bytes padding
    glm::vec3 Kd;       // 12 bytes
    float _pad1;                 // 4 bytes padding
    glm::vec3 Ld;       // 12 bytes
    float _pad2;                 // 4 bytes padding
    glm::vec3 camera_position;
    float _pad3;
    // Default constructor - zero initialize
    FrameUniformsStd140()
        : view_projection(1.0f)
        , light_position(0.0f), _pad0(0.0f)
        , Kd(0.0f), _pad1(0.0f)
        , Ld(0.0f), _pad2(0.0f) {}

    // Convert from convenient ObjectUniforms to std140 layout
    static FrameUniformsStd140 from(const FrameUniforms& src) {
        FrameUniformsStd140 dst;
        dst.view_projection = src.view_projection;
        dst.light_position = src.light_position;
        dst.Kd = src.Kd;
        dst.Ld = src.Ld;
        dst.camera_position = src.camera_position;
        return dst;
    }

    // Convert back from std140 to convenient ObjectUniforms (if needed)
    static FrameUniforms toObjectUniforms(const FrameUniformsStd140& src) {
        FrameUniforms dst;
        dst.view_projection = src.view_projection;
        dst.light_position = src.light_position;
        dst.Kd = src.Kd;
        dst.Ld = src.Ld;
        dst.camera_position = src.camera_position;
        return dst;
    }
};

// ============================================================================
// Per-Material Uniforms (Descriptor Set 1)
// Shared across all instances of the same material type
// Updated when material properties change
// ============================================================================

struct MaterialUniforms {
    UniformMap uniforms;
};

// ============================================================================
// Per-Object Uniforms (Descriptor Set 2 or Push Constants)
// Unique for each object instance, updated every frame
// ============================================================================

// Convenient C++ struct for working with object uniforms in code
struct ObjectUniforms {
  glm::mat4 model_matrix = glm::mat4(1.0f);
  glm::mat3 normal_matrix = glm::mat3(1.0f);
};

// std140-compatible struct for GPU buffer (112 bytes total)
// glm::mat3 in std140 is stored as 3 vec4 columns (each column aligned to 16 bytes)
struct alignas(16) ObjectUniformsStd140 {
    glm::mat4 model_matrix;      // 64 bytes
    glm::vec3 normal_col0;       // 12 bytes
    float _pad0;                 // 4 bytes padding
    glm::vec3 normal_col1;       // 12 bytes
    float _pad1;                 // 4 bytes padding
    glm::vec3 normal_col2;       // 12 bytes
    float _pad2;                 // 4 bytes padding
    // Total: 64 + 48 = 112 bytes

    // Default constructor - zero initialize
    ObjectUniformsStd140()
        : model_matrix(1.0f)
        , normal_col0(0.0f), _pad0(0.0f)
        , normal_col1(0.0f), _pad1(0.0f)
        , normal_col2(0.0f), _pad2(0.0f) {}

    // Convert from convenient ObjectUniforms to std140 layout
    static ObjectUniformsStd140 from(const ObjectUniforms& src) {
        ObjectUniformsStd140 dst;
        dst.model_matrix = src.model_matrix;
        dst.normal_col0 = glm::column(src.normal_matrix, 0);
        dst.normal_col1 = glm::column(src.normal_matrix, 1);
        dst.normal_col2 = glm::column(src.normal_matrix, 2);
        return dst;
    }

    // Convert back from std140 to convenient ObjectUniforms (if needed)
    static ObjectUniforms toObjectUniforms(const ObjectUniformsStd140& src) {
        ObjectUniforms dst;
        dst.model_matrix = src.model_matrix;
        dst.normal_matrix = glm::mat3(src.normal_col0, src.normal_col1, src.normal_col2);
        return dst;
    }
};

} // namespace Core::Uniforms
