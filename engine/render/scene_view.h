#pragma once

#include "render_item.h"

namespace ssme {

class SceneView {

public:
  float z = 0;
  float x = 0;
  std::vector<RenderItem> opaque_objects;
};
} // namespace ssme
