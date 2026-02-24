#pragma once

#include "drawing_policy.h"

namespace Render {

/**
 * @brief DefaultDrawingPolicy - standard drawing policy for default material type
 * 
 * Backend-agnostic: uses RHI abstractions (CommandList, DrawingData)
 * 
 * Expected descriptor sets:
 * - Set 0: Per-Frame (camera matrices, projection)
 * - Set 1: Per-Material (material textures, parameters)
 * - Set 2: Per-Object (model matrix)
 */
struct DefaultDrawingPolicy {
    /// Render function for default material
    static void render(CommandList& cmd, const DrawingData& data);

    /// Create drawing policy (pipeline comes from material at render time)
    [[nodiscard]] static DrawingPolicy create();
};

} // namespace Render
