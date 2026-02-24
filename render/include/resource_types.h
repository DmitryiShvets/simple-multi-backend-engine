#pragma once

#include "rid.h"
#include "uniform_set.h"
#include "uniform_value.h"
#include "uniform_layout.h"
#include <string>

namespace Render {

// --- Core Types ---

using RID = Core::RID;
using UniformValue = Core::UniformValue;
using UniformMap = Core::UniformMap;
using UniformSet = Core::UniformSet;
using UniformLayout = Core::UniformLayout;
// --- Enums ---

enum class ResourceType {
  UNDEFINED,
  SWAP_CHAIN,
  TEXTURE,
  IMAGE,
  IMAGE_VIEW,
  BUFFER,
  PIPELINE,
  DESCRIPTOR_SET_LAYOUT,
  DESCRIPTOR_SET,       // Allocated descriptor set
  DESCRIPTOR_POOL,      // Descriptor pool that owns descriptor sets
  PIPELINE_LAYOUT,
  MATERIAL_TEMPLATE,
};

enum class ResourceState { UNDEFINED, TRANSFER_DST, PRESENT_SRC };

struct Material {
  std::string name;
  struct RenderData {
    RID pipeline;
    RID uniforms_buf;   // Material-level uniform buffer RID (albedo, roughness, etc.)
    RID uniforms_ds;    // Descriptor set for material-level resources
  } render_data;
};

} // namespace Render
