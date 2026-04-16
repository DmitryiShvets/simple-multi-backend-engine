#pragma once

#include "render_device.h"
#include "opengl_gpu_storage_fwd.h"

namespace ssme::opengl {

// This is the concrete OpenGL implementation of the pure RenderDevice interface.
class OpenGLRenderDevice final : public RenderDevice {
public:
    OpenGLRenderDevice(OpenGLGpuStorageMT& storage);
    virtual ~OpenGLRenderDevice() override;

    // --- Buffer ---
    RID createBuffer(const BufferDesc& desc, RID id = RID::INVALID) override;
    void destroyBuffer(RID rid) override;
    // --- Texture ---
    RID createTexture(const TextureDesc& desc, RID id = RID::INVALID) override;
    void destroyTexture(RID rid) override;
    // --- Sampler ---
    RID createSampler(const SamplerDesc& desc, RID id = RID::INVALID) override;
    // --- Descriptor layout ---
    RID createDescriptorLayout(const DescriptorLayout &desc, RID id = RID::INVALID) override;
    void destroyDescriptorLayout(RID id) override;
    // --- Descriptor ---
    RID createDescriptor(const DescriptorDesc &desc,
                                 RID id = RID::INVALID) override;
    void destroyDescriptor(RID id) override;
    // --- Pipeline layout ---
    RID createPipelineLayout(const PipelineLayoutDesc &desc, RID id = RID::INVALID) override;
    void destroyPipelineLayout(RID id) override;
    RID containsPipelineLayout(std::size_t hash) override;
    // --- Pipeline ---
    RID createGraphicsPipeline(const GraphicsPipelineDesc& desc, RID id = RID::INVALID) override;
    void destroyGraphicsPipeline(RID id) override;
    RID containsGraphicsPipeline(std::size_t hash) override;
    // --- Shader ---
    RID createShaderModule(const ShaderModuleDesc &desc, RID id = RID::INVALID) override;
    void destroyShaderModule(RID id) override;

    void updateBufferRaw(RID rid, size_t offset, size_t size, const void *data) override;

private:
    OpenGLGpuStorageMT& m_storage;
};

} // namespace ssme::opengl
