#pragma once

#include "render_device.h"

namespace Render {
    class PipelineConfigRegistry;
}

namespace Render::OpenGL {

class OpenglResourceManager; // Forward-declaration

// This is the concrete OpenGL implementation of the pure Device interface.
class OpenGLDevice final : public Device {
public:
    OpenGLDevice(OpenglResourceManager& resource_manager, PipelineConfigRegistry& pl_registry);
    virtual ~OpenGLDevice() override;

    // --- Device Interface Implementation ---

    RID createBuffer(const BufferDesc& desc) override;
    RID createTexture(const TextureDesc& desc) override;
    RID createSampler(const SamplerDesc& desc) override;
    RID createDescriptorSetLayout(const DescriptorSetLayoutDesc &desc) override;
    RID createDescriptorSet(RID layout_rid, const std::vector<RID>& buffer_rids) override;
    RID createPipelineLayout(const PipelineLayoutDesc &desc) override;
    RID createGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
    RID createPipeline(const PipelineDesc &desc) override;
    RID createMaterial(const std::string &mat_name, const UniformSet& material_uniforms) override;

    void updateBufferRaw(RID rid, size_t offset, size_t size, const void *data) override;

    void free(RID rid) override;

private:
    PipelineConfigRegistry& m_pl_registry;
    OpenglResourceManager& m_resource_manager;
};

} // namespace Render::OpenGL
