#pragma once

#include "core/resource_types.h"

#include <cstddef>
#include <string>

namespace ssme {

// Forward declaration
struct PipelineConfig;

// This is the main "factory" and "submission" interface for the GPU.
// It is completely API-agnostic.
class RenderDevice {
public:
  virtual ~RenderDevice() = default;

  // --- Buffer ---
  virtual RID createBuffer(const BufferDesc &desc, RID id = RID::INVALID) = 0;
  virtual void destroyBuffer(RID id) = 0;
  // --- Texture ---
  virtual RID createTexture(const TextureDesc &desc, RID id = RID::INVALID) = 0;
  virtual void destroyTexture(RID id) = 0;
  // --- Sampler ---
  virtual RID createSampler(const SamplerDesc &desc, RID id = RID::INVALID) = 0;
  // --- Descriptor layout ---
  virtual RID createDescriptorLayout(const DescriptorLayout &desc, RID id = RID::INVALID) = 0;
  virtual void destroyDescriptorLayout(RID id) = 0;
  // --- Descriptor ---
  virtual RID createDescriptor(const DescriptorDesc &desc, RID id = RID::INVALID) = 0;
  virtual void destroyDescriptor(RID id) = 0;
  // --- Pipeline layout ---
  virtual RID createPipelineLayout(const PipelineLayoutDesc &desc, RID id = RID::INVALID) = 0;
  virtual void destroyPipelineLayout(RID id) = 0;
  virtual RID containsPipelineLayout(std::size_t hash) = 0;
  // --- Pipeline ---
  virtual RID createGraphicsPipeline(const GraphicsPipelineDesc &desc, RID id = RID::INVALID) = 0;
  virtual void destroyGraphicsPipeline(RID id) = 0;
  virtual RID containsGraphicsPipeline(std::size_t hash) = 0;
  // --- Shader ---
  virtual RID createShaderModule(const ShaderModuleDesc &desc, RID id = RID::INVALID) = 0;
  virtual void destroyShaderModule(RID id) = 0;

  // Update buffer data at runtime
  virtual void updateBufferRaw(RID rid, size_t offset, size_t size,
                               const void *data) = 0;

  // Update uniform buffer data at runtime (full overwrite, type-safe)
  template <typename T> void updateBuffer(RID rid, const T &data);
};

template <typename T> void RenderDevice::updateBuffer(RID rid, const T &data) {
  updateBufferRaw(rid, 0, sizeof(T), &data);
}

} // namespace ssme
