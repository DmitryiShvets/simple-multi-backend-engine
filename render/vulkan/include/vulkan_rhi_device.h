#pragma once

#include "render_device.h"
#include <string>

namespace Render {
class PipelineConfigRegistry;
}

namespace Render::Vulkan {

class VulkanDevice;
class VulkanResourceManager;

class VulkanRHIDevice final : public RenderDevice {
public:
  VulkanRHIDevice(VulkanDevice &device, VulkanResourceManager &resource_manager,
                  PipelineConfigRegistry &pl_registry);
  virtual ~VulkanRHIDevice() override;

  // --- Resource Management ---
  RID createBuffer(const BufferDesc &desc) override;
  RID createTexture(const TextureDesc &desc) override;
  RID createSampler(const SamplerDesc &desc) override;
  RID createDescriptorSetLayout(const DescriptorSetLayoutDesc &desc) override;
  RID createDescriptorSet(RID layout_rid, const std::vector<RID>& buffer_rids) override;
  RID createPipelineLayout(const PipelineLayoutDesc &desc) override;
  RID createGraphicsPipeline(const GraphicsPipelineDesc &desc) override;
  //  RID createComputePipeline(const ComputePipelineDesc& desc) override;
  RID createPipeline(const PipelineDesc &desc) override;
  RID createMaterial(const std::string &mat_name, const UniformSet& material_uniforms) override;
  Material* getMaterial(RID material_rid) override;

  const PipelineConfig* getPipelineConfig(const std::string& material_name) const override;

  void updateBufferRaw(RID rid, size_t offset, size_t size, const void *data) override;

  void free(RID rid) override;

private:
  VulkanDevice &m_device;
  PipelineConfigRegistry &m_pl_registry;
  VulkanResourceManager &m_resource_manager;
};

} // namespace Render::Vulkan
