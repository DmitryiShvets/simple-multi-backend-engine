#pragma once
#include "render_types.h"
#include <glm/vec3.hpp>

namespace Render {

class PipelineConfigRegistry;

// DefaultMaterial template
struct DefaultPipeline {
    // Register material config in PipelineConfigRegistry
    static void addConfigToRegistry(PipelineConfigRegistry &registry, BackendType type);
};

} // namespace Render
