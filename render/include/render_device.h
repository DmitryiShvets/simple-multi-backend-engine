#pragma once

#include "command_list.h"
#include "resource_types.h"
#include <span>

namespace Render {

// This is the main "factory" and "submission" interface for the GPU.
// It is completely API-agnostic.
class Device {
public:
  virtual ~Device() = default;

  // --- Resource Management ---
  virtual RID createBuffer(const BufferDesc &desc) = 0;
  virtual RID createTexture(const TextureDesc &desc) = 0;
  virtual RID createSampler(const SamplerDesc &desc) = 0;
  virtual RID
  createDescriptorSetLayout(const DescriptorSetLayoutDesc &desc) = 0;
  virtual RID createPipelineLayout(const PipelineLayoutDesc &desc) = 0;
  virtual RID createGraphicsPipeline(const GraphicsPipelineDesc &desc) = 0;
  // virtual RID createComputePipeline(const ComputePipelineDesc& desc) = 0;
  virtual void free(RID rid) = 0;
};

} // namespace Render
