#pragma once
#include "rid.h"
#include "uniforms.h"
#include <string>

namespace Core::Ecs::Component {

// Runtime components - GPU resources created at runtime
// material_id: RID on material template (contains pipeline RID + material-level
// uniforms) obj_uniform_id: RID on uniform buffer with per-object data (model
// matrix, normal matrix)
struct VkRuntime {
  RID geometry_id;    // Vertex buffer RID (unique per object)
  RID material_id;    // Material template RID (shared across material type)
  RID obj_uniform_id; // Per-object uniform buffer RID (unique per object)
  RID obj_uniform_ds; // Descriptor set for object uniform (unique per object)
  std::string material_type; // Material type name for DrawingPolicy lookup
  bool visible = true;
  glm::mat4 model_matrix;
};

struct GlRuntime {
  RID geometry_id;
  RID material_id;
  RID obj_uniform_id;
  RID obj_uniform_ds;
  std::string material_type;
  bool visible = true;
  glm::mat4 model_matrix;
};

} // namespace Core::Ecs::Component
