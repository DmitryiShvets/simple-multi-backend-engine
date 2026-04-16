#pragma once

#include "render_item.h"
#include "core/resource_types.h"
#include "resource_handle.h"
#include "resources/descriptor_set.h"
#include "resources/pipeline.h"
#include "resources/uniform_block.h"

namespace ssme {

struct RenderProxy {
  // 1. Identification (link to logic)
  EntityID entity_id;
  ResourceHandle<Pipeline> pipeline_ref;
  ResourceHandle<UniformBuffer> obj_uniforms;
  ResourceHandle<DescriptorSet> obj_uniform_ds;
  ResourceHandle<DescriptorSetLayout> obj_uniform_ds_layout;
  // 2. GEOMETRY AND DATA
  RenderItem render_item;

  // 3. CULLING DATA
  // Local mesh AABB (doesn't change)
  AABB local_aabb;
  // World AABB (updated when Transform changes)
  AABB world_aabb;

  // 4. SORTING DATA (Optimization)
  // Material hash or Pipeline RID to minimize state switches
  uint64_t sorting_key = 0;

  // Distance to camera (computed in RenderModule before drawing)
  float depth = 0.0f;

  // 5. STATE
  bool is_visible = true;   // Result of last Frustum Culling
  bool cast_shadows = true; // Participates in Shadow Pass
  uint32_t layer_mask = 1; // Bitmask (e.g.: 1-Default, 2-UI, 4-Water)
};

} // namespace ssme
