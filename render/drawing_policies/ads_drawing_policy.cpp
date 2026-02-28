#include "ads_drawing_policy.h"
#include "render_types.h"
#include <cassert>

namespace Render {

void AdsDrawingPolicy::render(CommandList& cmd, const DrawingData& data) {
    // Validate required data
    assert(data.pipeline.isValid() && "DrawingData: pipeline is not valid");
    assert(data.vertex_buffer.isValid() && "DrawingData: vertex_buffer is not valid");
    assert(data.vertex_count > 0 && "DrawingData: vertex_count must be > 0");
    assert(data.getDescriptorSet(0).isValid() && "DrawingData: per-frame descriptor set (0) is required");
    assert(data.getDescriptorSet(1).isValid() && "DrawingData: material descriptor set (1) is required");
    assert(data.getDescriptorSet(2).isValid() && "DrawingData: object descriptor set (2) is required");
    assert(data.hasPushConstants() && "DrawingData: push_constants (model_mat) is required");

    // 0. Bind pipeline
    cmd.setGraphicsPipeline(data.pipeline);

    // 1. Bind vertex buffer
    cmd.setVertexBuffer(0, data.vertex_buffer, 0);

    // 2. Bind descriptor sets
    // Set 0: Per-Frame (camera matrices, projection)
    cmd.setDescriptorSet(0, data.getDescriptorSet(0), data.pipeline);
    // Set 1: Per-Material (material color)
    cmd.setDescriptorSet(1, data.getDescriptorSet(1), data.pipeline);
    // Set 2: Per-Object (normal matrix)
    cmd.setDescriptorSet(2, data.getDescriptorSet(2), data.pipeline);

    // 3. Set push constants (model matrix)
    auto it = data.push_constants.find("model_mat");
    assert(it != data.push_constants.end() && "DrawingData: model_mat push constant is required");
    cmd.setPushConstant(data.pipeline, it->second, static_cast<uint32_t>(ShaderStage::VERTEX));

    // 4. Draw (non-indexed)
    cmd.draw(data.vertex_count, data.instance_count, data.first_vertex, 0);
}

DrawingPolicy AdsDrawingPolicy::create() {
    return DrawingPolicy{.render_func = render};
}

} // namespace Render
