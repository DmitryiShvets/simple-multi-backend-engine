#pragma once

#include "rid.h"
#include <vector>

namespace Core {

class SceneView {

public:
  struct MeshRenderable {
    RID geometry_id;
    RID material_id;
  };

  std::vector<MeshRenderable> opaque_objects;
};
} // namespace Core
