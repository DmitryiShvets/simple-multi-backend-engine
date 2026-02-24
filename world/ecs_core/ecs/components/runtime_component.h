#pragma once
#include "rid.h"
#include "uniforms.h"
#include <string>

namespace Core::Ecs::Component {

// Runtime components - GPU resources created at runtime
// material_id: RID on material template (contains pipeline RID + material-level uniforms)
// data_id: RID on uniform buffer with per-object data (model matrix, etc.)
struct VkRuntime {
    RID geometry_id;            // Vertex buffer RID (unique per object)
    RID material_id;   // Material template RID (shared across material type)
    Uniforms::ObjectUniforms data_id;     // Per-object uniform buffer RID (unique per object)
    std::string material_type;  // Material type name for DrawingPolicy lookup
    bool visible = true;
};

struct GlRuntime {
    RID geometry_id;
    RID material_id;
    Uniforms::ObjectUniforms data_id;
    std::string material_type;
    bool visible = true;
};

} // namespace Core::Ecs::Component
