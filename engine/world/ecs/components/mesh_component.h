#pragma once
#include "resources/mesh.h"
#include "resource_handle.h"

namespace ssme {
struct MeshComponent {
  ResourceHandle<Mesh> handle;
};

} // namespace ssme
