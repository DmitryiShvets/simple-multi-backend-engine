#pragma once

#include "material_params.h"

namespace Core::Ecs::Component {

// Default material component
// Instance-level parameters stored per object
using DefaultMaterial = Core::MaterialParams::DefaultMaterial;
using PbrMaterial = Core::MaterialParams::PbrMaterialParams;
using AdsMaterial = Core::MaterialParams::AdsMaterial;

} // namespace Core::Ecs::Component
