#pragma once
#include "render_types.h"
#include <glm/vec3.hpp>

namespace Render {

class PipelineConfigRegistry;

// ADS Material template
struct AdsPipeline {
    // Register material config in PipelineConfigRegistry
    static void addConfigToRegistry(PipelineConfigRegistry &registry, BackendType type);
};

} // namespace Render
