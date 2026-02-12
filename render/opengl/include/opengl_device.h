#pragma once

#include "render_device.h"

namespace Render::OpenGL {

class OpenglResourceManager; // Forward-declaration

// This is the concrete OpenGL implementation of the pure Device interface.
class OpenGLDevice final : public Device {
public:
    OpenGLDevice(OpenglResourceManager& resource_manager);
    virtual ~OpenGLDevice() override;

    // --- Device Interface Implementation ---

    RID createBuffer(const BufferDesc& desc) override;
    RID createTexture(const TextureDesc& desc) override;
    RID createSampler(const SamplerDesc& desc) override;
    RID createDescriptorSetLayout(const DescriptorSetLayoutDesc &desc) override;
    RID createPipelineLayout(const PipelineLayoutDesc &desc) override;
    RID createGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
    void free(RID rid) override;

private:
    OpenglResourceManager& m_resource_manager;
};

} // namespace Render::OpenGL
