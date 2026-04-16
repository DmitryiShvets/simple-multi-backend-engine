#pragma once

#include "core/rid.h"
#include "core/uniform_value.h"

#include <cstdint>
#include <glm/fwd.hpp>

namespace ssme {
using EntityID = uint64_t;

struct DrawCommand {
  /// Number of vertices to draw
  uint32_t vertex_count = 0;
  /// Number of instances (1 for non-instanced drawing)
  uint32_t instance_count = 1;
  /// First vertex index in the vertex buffer
  uint32_t first_vertex = 0;
  uint32_t first_instance = 0;

  /// Number of indices to draw (used only if index_buffer is valid) if
  /// index_count > 0, use DrawIndexed
  uint32_t index_count = 0;
  /// First index in the index buffer
  uint32_t first_index = 0;
  int32_t vertex_offset = 0;
};
/**
 * @brief Universal data container for rendering a single object
 *
 * Designed to work with any Mesh/Material. Provides all necessary data
 * for rendering: descriptor sets, geometry, push constants, pipeline.
 *
 * Descriptor Set Layout:
 * - Set 0: Per-Frame data (camera matrices, projection, time)
 * - Set 1: Per-Object data (model matrix, object-specific uniforms)
 * - Set 2: Per-Material data (textures, material parameters)
 *
 * drawing_policy.render(cmd, draw_data);
 * @endcode
 */
struct RenderItem {
  // ========================================================================
  // Common
  // ========================================================================
  EntityID ent;
  DrawCommand draw_cmd;

  // ========================================================================
  // Geometry
  // ========================================================================

  /// Vertex buffer RID
  std::vector<RID> vertex_buffers;
  /// Index buffer RID (optional, for indexed drawing)
  RID index_buffer = RID::INVALID;

  // ========================================================================
  // Descriptor Sets
  // ========================================================================

  /**
   * @brief Descriptor sets for binding
   *
   * Often it has:
   * - [0] Per-Frame: camera matrices, projection, view, time
   * - [1] Per-Material: textures, material parameters (albedo, roughness)
   * - [2] Per-Object: model matrix, object-specific data
   */
  std::vector<RID> descriptor_sets;

  // ========================================================================
  // Push Constants
  // ========================================================================

  /**
   * @brief Push constants for shader
   *#include "
   * Always present but can be empty. Use for small frequently-updated data:
   * - Tessellation levels
   * - Object flags
   * - Small transformation parameters
   */
  UniformMap push_constants;

  // ========================================================================
  // Pipeline
  // ========================================================================

  /// Graphics pipeline RID (from material template)
  RID pipeline = RID::INVALID;

  // ========================================================================
  // Helper Methods
  // ========================================================================

  /**
   * @brief Set descriptor set for a specific binding point
   * @param set_index
   * @param set_rid Descriptor set RID
   */
  void setDescriptor(uint32_t set_index, RID set_rid) {
    if (set_index >= descriptor_sets.size()) {
      descriptor_sets.resize(set_index + 1, RID::INVALID);
    }
    descriptor_sets[set_index] = set_rid;
  }

  /**
   * @brief Get descriptor set for a specific binding point
   * @param set_index
   * @return Descriptor set RID
   */
  [[nodiscard]] RID getDescriptor(uint32_t set_index) const {
    return (set_index < descriptor_sets.size()) ? descriptor_sets[set_index]
                                                : RID::INVALID;
  }

  /**
   * @brief Check if indexed drawing should be used
   */
  [[nodiscard]] bool isIndexed() const {
    return index_buffer.isValid() && draw_cmd.index_count > 0;
  }

  /**
   * @brief Check if instanced drawing should be used
   */
  [[nodiscard]] bool isInstanced() const { return draw_cmd.instance_count > 1; }

  /**
   * @brief Check if push constants are provided (non-empty)
   */
  [[nodiscard]] bool hasPushConstants() const {
    return !push_constants.empty();
  }
};

} // namespace ssme
