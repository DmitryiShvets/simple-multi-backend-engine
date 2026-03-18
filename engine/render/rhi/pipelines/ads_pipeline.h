#pragma once
#include "core/gpu_types.h"
#include <glm/vec3.hpp>

namespace ssme {

class PipelineConfigRegistry;

// ADS Material template
struct AdsPipeline {
    // Register material config in PipelineConfigRegistry
    static void addConfigToRegistry(PipelineConfigRegistry &registry, GpuBackend type);
};

} // namespace ssme
