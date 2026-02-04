#pragma once

#include "render_device.h"

namespace Render::OpenGL {

// This is the concrete OpenGL implementation of the pure Device interface.
class OpenGLDevice final : public Device {
public:
    OpenGLDevice();
    virtual ~OpenGLDevice() override;

    // --- Device Interface Implementation ---

    RID createBuffer(const BufferDesc& desc) override;
    RID createTexture(const TextureDesc& desc) override;
    RID createSampler(const SamplerDesc& desc) override;
    RID createGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
    void free(RID rid) override;

    CommandList* beginCommandList() override;
    void submitCommandLists(std::span<CommandList*> lists) override;

    void waitIdle() override;
    void tick() override;
};

} // namespace Render::OpenGL
