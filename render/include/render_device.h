#pragma once

#include "types.h"
#include "command_list.h"
#include <span>

namespace Render {

// This is the main "factory" and "submission" interface for the GPU.
// It is completely API-agnostic.
class Device {
public:
    virtual ~Device() = default;

    // --- Resource Management ---
    virtual RID createBuffer(const BufferDesc& desc) = 0;
    virtual RID createTexture(const TextureDesc& desc) = 0;
    virtual RID createSampler(const SamplerDesc& desc) = 0;
    virtual RID createGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;
    // virtual RID createComputePipeline(const ComputePipelineDesc& desc) = 0;
    virtual void free(RID rid) = 0;

    // --- Command Execution ---
    virtual CommandList* beginCommandList() = 0;
    virtual void submitCommandLists(std::span<CommandList*> lists) = 0;

    // --- Maintenance & Synchronization ---
    virtual void tick() = 0;
    virtual void waitIdle() = 0;
};

} // namespace Render
