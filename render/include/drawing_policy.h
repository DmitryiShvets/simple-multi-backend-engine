#pragma once

#include "command_list.h"
#include "drawing_data.h"

namespace Render {

/**
 * @brief DrawingPolicy - render function for a material type
 * 
 * Does NOT store pipeline - pipeline comes from material template at render
 * time. Value semantics - copied by value, no vtable lookup.
 * 
 * Each drawing policy defines how to render objects with a specific material
 * type, using the universal DrawingData structure.
 */
struct DrawingPolicy {
    using RenderFunc = void (*)(CommandList& cmd, const DrawingData& params);

    RenderFunc render_func;  ///< Render function pointer

    /// Render using this policy
    void render(CommandList& cmd, const DrawingData& params) const {
        render_func(cmd, params);
    }
};

} // namespace Render
