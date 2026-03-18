#pragma once

#include "core/rid.h"
#include "core/uniform_value.h"

#include <array>
#include <cstdint>

namespace ssme {

/**
 * @brief Universal data container for rendering a single object
 *
 * Designed to work with any DrawingPolicy. Provides all necessary data
 * for rendering: descriptor sets, geometry, push constants, pipeline.
 *
 * Descriptor Set Layout:
 * - Set 0: Per-Frame data (camera matrices, projection, time)
 * - Set 1: Per-Material data (textures, material parameters)
 * - Set 2: Per-Object data (model matrix, object-specific uniforms)
 *
 * drawing_policy.render(cmd, draw_data);
 * @endcode
 */
struct DrawingData {
  // ========================================================================
  // Descriptor Sets
  // ========================================================================

  /**
   * @brief Descriptor sets for binding
   *
   * Index mapping:
   * - [0] Per-Frame: camera matrices, projection, view, time
   * - [1] Per-Material: textures, material parameters (albedo, roughness)
   * - [2] Per-Object: model matrix, object-specific data
   */
  std::array<RID, 3> descriptor_sets = {RID::INVALID, RID::INVALID,
                                              RID::INVALID};

  // ========================================================================
  // Geometry
  // ========================================================================

  /// Vertex buffer RID
  RID vertex_buffer = RID::INVALID;
  /// Index buffer RID (optional, for indexed drawing)
  RID index_buffer = RID::INVALID;
  /// Number of vertices to draw
  uint32_t vertex_count = 0;
  /// Number of indices to draw (used only if index_buffer is valid)
  uint32_t index_count = 0;
  /// First vertex index in the vertex buffer
  uint32_t first_vertex = 0;
  /// First index in the index buffer
  uint32_t first_index = 0;
  /// Number of instances (1 for non-instanced drawing)
  uint32_t instance_count = 1;

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
   * @param set_index 0, 1, or 2
   * @param set_rid Descriptor set RID
   */
  void setDescriptorSet(uint32_t set_index, RID set_rid) {
    if (set_index < 3) {
      descriptor_sets[set_index] = set_rid;
    }
  }

  /**
   * @brief Get descriptor set for a specific binding point
   * @param set_index 0, 1, or 2
   * @return Descriptor set RID
   */
  [[nodiscard]] RID getDescriptorSet(uint32_t set_index) const {
    return (set_index < 3) ? descriptor_sets[set_index] : RID::INVALID;
  }

  /**
   * @brief Check if indexed drawing should be used
   */
  [[nodiscard]] bool isIndexed() const {
    return index_buffer.isValid() && index_count > 0;
  }

  /**
   * @brief Check if instanced drawing should be used
   */
  [[nodiscard]] bool isInstanced() const { return instance_count > 1; }

  /**
   * @brief Check if push constants are provided (non-empty)
   */
  [[nodiscard]] bool hasPushConstants() const {
    return !push_constants.empty();
  }
};

} // namespace ssme
