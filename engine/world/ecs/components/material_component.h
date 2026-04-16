#pragma once
#include "resources/material.h"
#include "resource_handle.h"

namespace ssme {
struct MaterialComponent {
  ResourceHandle<Material> handle;
};
} // namespace ssme
