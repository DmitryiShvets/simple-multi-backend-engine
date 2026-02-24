#pragma once

#include "render_device.h"
#include <string>

namespace Render {
class PipelineConfigRegistry;
}

namespace Render::Vulkan {

class VulkanDevice;
class VulkanResourceManager;

// This class is the concrete Vulkan implementation of the pure Device interface
// (Level 5). Its name is changed to reflect its role.
class VulkanRHIDevice final : public Device {
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

  void updateBufferRaw(RID rid, size_t offset, size_t size, const void *data) override;

  void free(RID rid) override;

private:
  VulkanDevice &m_device;
  PipelineConfigRegistry &m_pl_registry;
  VulkanResourceManager &m_resource_manager;
};

} // namespace Render::Vulkan
