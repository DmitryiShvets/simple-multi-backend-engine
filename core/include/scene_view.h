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
    Uniforms::ObjectUniforms data_id;            // Per-object uniform buffer RID (model matrix, etc.)
  };

  std::vector<MeshRenderable> opaque_objects;
};
} // namespace Core
