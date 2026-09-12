#pragma once

#include "camera.h"
#include "render_item.h"

#include <memory>

namespace ssme {

class SceneView {

public:
  float z = 0;
  float x = 0;
  std::vector<RenderItem> opaque_objects;
  std::shared_ptr<Camera> camera;
};
} // namespace ssme
