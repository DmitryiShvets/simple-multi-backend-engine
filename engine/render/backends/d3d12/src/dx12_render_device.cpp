#include "dx12_render_device.h"
#include "core/rid.h"
#include "dx12_gpu_storage_fwd.h"
#include "utils/debug_assert.h"

#include "core/resource_types.h"

#include <cstddef>

namespace ssme::d3d12 {

Dx12RenderDevice::Dx12RenderDevice(Dx12GpuStorageMT &storage)
    : m_storage(storage) {}

Dx12RenderDevice::~Dx12RenderDevice() {}

//------------------------------------------------------------------------
// ---------------------------- Buffer -----------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createBuffer(const BufferDesc &desc, RID id) {
  debug_assert(desc.size > 0, "Buffer size cannot be zero");

  return RID::INVALID;
}

void Dx12RenderDevice::destroyBuffer(RID rid) {
  if (rid.isNull())
    return;
}

//------------------------------------------------------------------------
// ---------------------------- Texture ----------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createTexture(const TextureDesc &desc, RID id) {
  return RID::INVALID;
}

void Dx12RenderDevice::destroyTexture(RID rid) {
  if (rid.isNull())
    return;
  // TODO: Implement when textures are added
}

//------------------------------------------------------------------------
// ---------------------------- Sampler ----------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createSampler(const SamplerDesc &desc, RID id) {
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Descriptor layout ------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createDescriptorLayout(const DescriptorLayout &desc,
                                             RID id) {
  return RID::INVALID;
}

void Dx12RenderDevice::destroyDescriptorLayout(RID id) {
  if (id.isNull())
    return;
}

//------------------------------------------------------------------------
// ---------------------------- Descriptor -------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createDescriptor(const DescriptorDesc &desc, RID id) {
  return RID::INVALID;
}

void Dx12RenderDevice::destroyDescriptor(RID id) {
  if (id.isNull())
    return;
}

//------------------------------------------------------------------------
// ---------------------------- Pipeline layout --------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createPipelineLayout(const PipelineLayoutDesc &desc,
                                           RID id) {
  return RID::INVALID;
}

void Dx12RenderDevice::destroyPipelineLayout(RID id) {}
RID Dx12RenderDevice::containsPipelineLayout(std::size_t hash) {
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Pipeline ---------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createGraphicsPipeline(const GraphicsPipelineDesc &desc,
                                             RID id) {
  return RID::INVALID;
}

void Dx12RenderDevice::destroyGraphicsPipeline(RID id) {
  if (id.isNull())
    return;
}

RID Dx12RenderDevice::containsGraphicsPipeline(std::size_t hash) {
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Shader -----------------------------------
// -----------------------------------------------------------------------

RID Dx12RenderDevice::createShaderModule(const ShaderModuleDesc &desc, RID id) {
  return RID::INVALID;
}

void Dx12RenderDevice::destroyShaderModule(RID id) {
  if (id.isNull())
    return;
}

//------------------------------------------------------------------------
// ---------------------------- Common -----------------------------------
// -----------------------------------------------------------------------

void Dx12RenderDevice::updateBufferRaw(RID rid, size_t offset, size_t size,
                                       const void *data) {}

} // namespace ssme::d3d12
