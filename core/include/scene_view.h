#pragma once

#include "rid.h"
#include "uniforms.h"
#include <vector>

namespace Core {

class SceneView {

public:
  struct MeshRenderable {
    RID geometry_id;        // Vertex buffer RID
    RID material_id;        // Material template RID (contains pipeline + material-level uniforms)
    RID obj_uniform_id;     // Per-object uniform buffer RID (model matrix, normal matrix)
    RID obj_uniform_ds;     // Descriptor set for object uniform
    glm::mat4 model_matrix;
  };
  float z;
  std::vector<MeshRenderable> opaque_objects;
};
} // namespace Core
