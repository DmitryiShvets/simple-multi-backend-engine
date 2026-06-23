#include "dx12_command_list.h"
#include "dx12_gpu_storage.h"
#include "dx12_helpers.h"
#include "com_exception.h"
#include "dx12_device.h"
#include <d3d12.h>
// #include <dxgi1_6.h>
namespace ssme::d3d12 {

Dx12CommandList::Dx12CommandList(Dx12Device &device, Dx12GpuStorageMT &storage)
    : m_device(device), m_storage(storage) {
  DX::ThrowIfFailed(m_device.getHandle()->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator)));
  // Create the command list.
  DX::ThrowIfFailed(m_device.getHandle()->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator.Get(), nullptr,
      IID_PPV_ARGS(&m_command_buffer)));

  // Command lists are created in the recording state, but there is nothing
  // to record yet. The main loop expects it to be closed, so close it now.
  DX::ThrowIfFailed(m_command_buffer->Close());
}

void Dx12CommandList::begin() {
  m_command_allocator->Reset();
  m_command_buffer->Reset(m_command_allocator.Get(), nullptr);
}
void Dx12CommandList::end() { m_command_buffer->Close(); }

// --- Pipeline State ---
void Dx12CommandList::setGraphicsPipeline(RID pipeline_rid) {}

void Dx12CommandList::setViewport(const Viewport &viewport) {}

void Dx12CommandList::setScissor(const Rect &rect) {}

void Dx12CommandList::setDepthBias(float constant_factor, float slope_factor) {
  if (constant_factor != 0.0f || slope_factor != 0.0f) {
  } else {
  }
}

// --- Resource Binding ---
void Dx12CommandList::setVertexBuffer(uint32_t first_binding, RID buffer_rid,
                                      uint64_t offset) {}

void Dx12CommandList::setIndexBuffer(RID buffer_rid, uint64_t offset,
                                     IndexType type) {}

void Dx12CommandList::setDescriptorSet(uint32_t set_index, RID set_rid,
                                       RID pipeline_rid) {
  (void)set_index;
  (void)pipeline_rid;

  // Get DescriptorSet and bind all resources
}

void Dx12CommandList::setPushConstant(RID pipeline_rid,
                                      const UniformValue &value,
                                      ShaderStageFlags stages,
                                      uint32_t offset) {}

// --- Drawing ---
void Dx12CommandList::draw(uint32_t vertex_count, uint32_t instance_count,
                           uint32_t first_vertex, uint32_t first_instance) {
  if (instance_count > 1) {
  } else {
  }
}

void Dx12CommandList::drawIndexed(uint32_t index_count, uint32_t instance_count,
                                  uint32_t first_index, int32_t vertex_offset,
                                  uint32_t first_instance) {}

void Dx12CommandList::drawIndexedIndirect(RID buffer_rid, uint64_t offset,
                                          uint32_t draw_count,
                                          uint32_t stride) {
  (void)buffer_rid;
  (void)offset;
  (void)draw_count;
  (void)stride;
}

// --- Compute ---
void Dx12CommandList::dispatch(uint32_t group_count_x, uint32_t group_count_y,
                               uint32_t group_count_z) {
  (void)group_count_x;
  (void)group_count_y;
  (void)group_count_z;
}

// --- Synchronization ---
void Dx12CommandList::pipelineBarrier(const BarrierInfo &barrier) {
    for (const auto &img_barrier : barrier.image_barriers) {
        auto texture = m_storage.get<Dx12Texture>(img_barrier.image);
        if (!texture) continue;

        D3D12_RESOURCE_BARRIER desc = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Transition = {
                .pResource = texture->getHandle().Get(),
                .StateBefore = toD3d12State(img_barrier.old_layout),
                .StateAfter  = toD3d12State(img_barrier.new_layout),
            },
        };
        m_command_buffer->ResourceBarrier(1, &desc);
    }
}

// --- Render Pass Management ---
void Dx12CommandList::beginRendering(const RenderingInfo &info) { (void)info; }

void Dx12CommandList::endRendering() {}

// --- Resource Manipulation ---
void Dx12CommandList::copyBuffer(RID src, RID dst, const BufferCopy &region) {
  (void)src;
  (void)dst;
  (void)region;
}

void Dx12CommandList::copyBufferToImage(RID src_buffer, RID dst_image,
                                        const BufferImageCopy &region) {
  (void)src_buffer;
  (void)dst_image;
  (void)region;
}

void Dx12CommandList::clearColorImage(RID image, const float color[4]) {
  (void)image;
}

} // namespace ssme::d3d12
