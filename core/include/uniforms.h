#pragma once

#include "uniform_value.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace Core::Uniforms {

// ============================================================================
// Per-Frame Uniforms (Descriptor Set 0)
// Updated once per frame, shared across all objects
// ============================================================================
struct FrameUniforms {
  glm::mat4 view_projection = glm::mat4(1.0f);
  glm::vec3 camera_position = glm::vec3(0.0f);
  float padding = 0.0f; // Alignment padding
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
struct ObjectUniforms {
  glm::mat4 model_matrix = glm::mat4(1.0f);
};

} // namespace Core::Uniforms
