#pragma once

#include "render_device.h"
#include "opengl_gpu_storage_fwd.h"

namespace ssme {
    class PipelineConfigRegistry;
}

namespace ssme::opengl {

// This is the concrete OpenGL implementation of the pure RenderDevice interface.
class OpenGLDevice final : public RenderDevice {
public:
    OpenGLDevice(OpenGLGpuStorageMT& storage, PipelineConfigRegistry& pl_registry);
    virtual ~OpenGLDevice() override;

    // --- RenderDevice Interface Implementation ---

    RID createBuffer(const BufferDesc& desc, RID id = RID::INVALID) override;
    void destroyBuffer(RID rid) override;
    RID createTexture(const TextureDesc& desc) override;
    void destroyTexture(RID rid) override;
    RID createSampler(const SamplerDesc& desc) override;
    RID createDescriptorSetLayout(const DescriptorSetLayoutDesc &desc) override;
    RID createDescriptorSet(RID layout_rid, const std::vector<RID>& buffer_rids) override;
    RID createPipelineLayout(const PipelineLayoutDesc &desc) override;
    RID createGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
    RID createPipeline(const PipelineDesc &desc) override;
    RID createMaterial(const std::string &mat_name, const UniformSet& material_uniforms) override;
    Material* getMaterial(RID material_rid) override;

    const PipelineConfig* getPipelineConfig(const std::string& material_name) const override;

    void updateBufferRaw(RID rid, size_t offset, size_t size, const void *data) override;

    void free(RID rid) override;

private:
    OpenGLGpuStorageMT& m_storage;
    PipelineConfigRegistry& m_pl_registry;
};

} // namespace ssme::opengl
