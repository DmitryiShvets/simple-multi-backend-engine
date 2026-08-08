#pragma once

#include "core/rid.h"
#include "core/render_types.h"
#include "core/resource_types.h"

namespace ssme {

// This is the abstract "drawing tablet" or "command recorder" for the RHI.
// It contains the set of all possible commands that can be recorded and
// later executed by the GPU.
class CommandList {
public:
    virtual ~CommandList() = default;

    // === Lifecycle ===
    virtual void begin() = 0;
    virtual void end() = 0;

    // === Pipeline State ===
    virtual void setGraphicsPipeline(RID pipeline_rid) = 0;
    virtual void setComputePipeline(RID pipeline_rid) = 0;

    virtual void setViewport(const Viewport& viewport) = 0;
    virtual void setScissor(const Rect& rect) = 0;
    virtual void setDepthBias(float constant_factor, float slope_factor) = 0;

    // === Resource Binding ===
    virtual void setVertexBuffer(uint32_t first_binding, RID buffer_rid, uint64_t offset) = 0;
    virtual void setIndexBuffer(RID buffer_rid, uint64_t offset, IndexType type) = 0;
    virtual void setDescriptorSet(uint32_t set_index, RID set_rid, RID pipeline_rid) = 0;
    virtual void setPushConstant(RID pipeline_rid, const UniformValue &value, ShaderStageFlags stages, uint32_t offset = 0) = 0;

    // === Drawing ===
    virtual void draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) = 0;
    virtual void drawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) = 0;
    virtual void drawIndexedIndirect(RID buffer_rid, uint64_t offset, uint32_t draw_count, uint32_t stride) = 0;

    // === Compute ===
    virtual void dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) = 0;

    // === Synchronization ===
    virtual void pipelineBarrier(const BarrierInfo& barrier) = 0;

    // === Render Pass Management ===
    virtual void beginRendering(const RenderingInfo& info) = 0;
    virtual void endRendering() = 0;

    // === Resource Manipulation ===
    virtual void copyBuffer(RID src, RID dst, const BufferCopy& region) = 0;
    virtual void copyBufferToImage(RID src_buffer, RID dst_image, const BufferImageCopy& region) = 0;
    virtual void clearColorImage(RID image, const float color[4]) = 0;
    virtual void blitImage(RID src, RID dst, const ImageBlit& region) = 0;
};

} // namespace ssme
