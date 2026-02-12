#pragma once

#include "render_device.h"

namespace Render::Vulkan {

class VulkanDevice;
class VulkanResourceManager;

// This class is the concrete Vulkan implementation of the pure Device interface
// (Level 5). Its name is changed to reflect its role.
class VulkanRHIDevice final : public Device {
public:
  VulkanRHIDevice(VulkanDevice &device,
                  VulkanResourceManager &resource_manager);
  virtual ~VulkanRHIDevice() override;

  // --- Resource Management ---
  RID createBuffer(const BufferDesc &desc) override;
  RID createTexture(const TextureDesc &desc) override;
  RID createSampler(const SamplerDesc &desc) override;
  RID createDescriptorSetLayout(const DescriptorSetLayoutDesc &desc) override;
  RID createPipelineLayout(const PipelineLayoutDesc &desc) override;
  RID createGraphicsPipeline(const GraphicsPipelineDesc &desc) override;
  //  RID createComputePipeline(const ComputePipelineDesc& desc) override;
  void free(RID rid) override;

private:
  VulkanDevice &m_device;
  VulkanResourceManager &m_resource_manager;
};

} // namespace Render::Vulkan
