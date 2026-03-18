#pragma once

#include "core/resource_types.h"
#include "core/uniform_set.h"

#include <string>

namespace ssme {

// Forward declaration
struct PipelineConfig;

// This is the main "factory" and "submission" interface for the GPU.
// It is completely API-agnostic.
class RenderDevice {
public:
  virtual ~RenderDevice() = default;

  // --- Resource Management ---
  virtual RID createBuffer(const BufferDesc &desc) = 0;
  virtual RID createTexture(const TextureDesc &desc) = 0;
  virtual RID createSampler(const SamplerDesc &desc) = 0;
  virtual RID
  createDescriptorSetLayout(const DescriptorSetLayoutDesc &desc) = 0;
  virtual RID createDescriptorSet(RID layout_rid,
                                  const std::vector<RID> &buffer_rids) = 0;
  virtual RID createPipelineLayout(const PipelineLayoutDesc &desc) = 0;
  virtual RID createGraphicsPipeline(const GraphicsPipelineDesc &desc) = 0;
  // virtual RID createComputePipeline(const ComputePipelineDesc& desc) = 0;
  virtual RID createPipeline(const PipelineDesc &desc) = 0;
  virtual RID createMaterial(const std::string &mat_name,
                             const UniformSet &material_uniforms) = 0;
  virtual Material *getMaterial(RID material_rid) = 0;

  // Get pipeline config by name (for accessing uniform layouts)
  virtual const PipelineConfig *
  getPipelineConfig(const std::string &material_name) const = 0;

  // Update buffer data at runtime
  virtual void updateBufferRaw(RID rid, size_t offset, size_t size,
                               const void *data) = 0;

  // Update uniform buffer data at runtime (full overwrite, type-safe)
  template <typename T> void updateBuffer(RID rid, const T &data);

  virtual void free(RID rid) = 0;
};

template <typename T> void RenderDevice::updateBuffer(RID rid, const T &data) {
  updateBufferRaw(rid, 0, sizeof(T), &data);
}

} // namespace ssme
