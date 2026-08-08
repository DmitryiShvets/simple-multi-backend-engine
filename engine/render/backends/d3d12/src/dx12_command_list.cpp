#include "dx12_command_list.h"
#include "com_exception.h"
#include "dx12_device.h"
#include "dx12_gpu_storage.h"
#include "dx12_helpers.h"
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
  std::wstring bufferName = L"CommandList111";
  m_command_buffer->SetName(bufferName.c_str());
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
void Dx12CommandList::setGraphicsPipeline(RID pipeline_rid) {
  auto *pipeline = m_storage.get<Dx12Pipeline>(pipeline_rid);
  if (!pipeline)
    return;
  m_command_buffer->SetPipelineState(pipeline->getHandle().Get());
  m_command_buffer->SetGraphicsRootSignature(pipeline->getLayoutHandle().Get());
}

void Dx12CommandList::setViewport(const Viewport &viewport) {
  D3D12_VIEWPORT vp = {
      .TopLeftX = viewport.x,
      .TopLeftY = viewport.y,
      .Width = viewport.width,
      .Height = viewport.height,
      .MinDepth = viewport.minDepth,
      .MaxDepth = viewport.maxDepth,
  };
  m_command_buffer->RSSetViewports(1, &vp);
}

void Dx12CommandList::setScissor(const Rect &rect) {
  D3D12_RECT r = {
      .left = rect.x,
      .top = rect.y,
      .right = rect.x + static_cast<LONG>(rect.width),
      .bottom = rect.y + static_cast<LONG>(rect.height),
  };
  m_command_buffer->RSSetScissorRects(1, &r);
}

void Dx12CommandList::setDepthBias(float constant_factor, float slope_factor) {
  if (constant_factor != 0.0f || slope_factor != 0.0f) {
  } else {
  }
}

// --- Resource Binding ---
void Dx12CommandList::setVertexBuffer(uint32_t first_binding, RID buffer_rid,
                                      uint64_t offset) {
  auto *buf = m_storage.get<Dx12Buffer>(buffer_rid);
  D3D12_VERTEX_BUFFER_VIEW vbv = {
      .BufferLocation = buf->getAddress() + offset,
      .SizeInBytes = (UINT)buf->getBufferSize(),
      .StrideInBytes = (UINT)buf->getInstanceSize(),
  };
  m_command_buffer->IASetVertexBuffers(first_binding, 1, &vbv);
}

void Dx12CommandList::setIndexBuffer(RID buffer_rid, uint64_t offset,
                                     IndexType type) {
  auto *buf = m_storage.get<Dx12Buffer>(buffer_rid);
  D3D12_INDEX_BUFFER_VIEW ibv = {
      .BufferLocation = buf->getAddress() + offset,
      .SizeInBytes = (UINT)buf->getBufferSize(),
      .Format = (type == IndexType::UINT16) ? DXGI_FORMAT_R16_UINT
                                            : DXGI_FORMAT_R32_UINT,
  };
  m_command_buffer->IASetIndexBuffer(&ibv);
}

void Dx12CommandList::setDescriptorSet(uint32_t set_index, RID set_rid,
                                       RID pipeline_rid) {
  auto *ds = m_storage.get<Dx12DescriptorSet>(set_rid);
  auto *pipeline = m_storage.get<Dx12Pipeline>(pipeline_rid);
  auto *layout = m_storage.get<Dx12PipelineLayout>(pipeline->getLayoutRID());

  auto &param_indices = layout->getParamIndices(set_index);
  for (size_t i = 0; i < ds->getBufferCount(); i++) {
    auto *buffer = m_storage.get<Dx12Buffer>(ds->getBufferRID(i));
    m_command_buffer->SetGraphicsRootConstantBufferView(param_indices[i],
                                                        buffer->getAddress());
  }

  // Get DescriptorSet and bind all resources
}

void Dx12CommandList::setPushConstant(RID pipeline_rid,
                                      const UniformValue &value,
                                      ShaderStageFlags stages,
                                      uint32_t offset) {
  auto *pipeline = m_storage.get<Dx12Pipeline>(pipeline_rid);
  if (!pipeline)
    return;
  auto *layout = m_storage.get<Dx12PipelineLayout>(pipeline->getLayoutRID());
  if (!layout || !layout->hasPushConstant())
    return;

  const uint32_t *data = static_cast<const uint32_t *>(value.data());
  uint32_t num_dwords = static_cast<uint32_t>(value.size() / 4);
  m_command_buffer->SetGraphicsRoot32BitConstants(
      layout->getPushConstantParamIndex(), num_dwords, data,
      offset / 4); // offset в DWORD'ах
}

// --- Drawing ---
void Dx12CommandList::draw(uint32_t vertex_count, uint32_t instance_count,
                           uint32_t first_vertex, uint32_t first_instance) {
  m_command_buffer->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  m_command_buffer->DrawInstanced(vertex_count, instance_count, first_vertex,
                                  first_instance);
}

void Dx12CommandList::drawIndexed(uint32_t index_count, uint32_t instance_count,
                                  uint32_t first_index, int32_t vertex_offset,
                                  uint32_t first_instance) {
  m_command_buffer->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  m_command_buffer->DrawIndexedInstanced(
      index_count, instance_count, first_index, vertex_offset, first_instance);
}

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
    if (!texture)
      continue;
    D3D12_RESOURCE_BARRIER desc = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Transition =
            {
                .pResource = texture->getHandle().Get(),
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = toD3d12State(img_barrier.old_layout),
                .StateAfter = toD3d12State(img_barrier.new_layout),
            },
    };
    m_command_buffer->ResourceBarrier(1, &desc);
  }
}

// --- Render Pass Management ---
void Dx12CommandList::beginRendering(const RenderingInfo &info) {
  if (info.color_attachments.empty())
    return;
  auto *texture = m_storage.get<Dx12Texture>(info.color_attachments[0].texture);
  if (!texture)
    return;
  auto rtvHandle = texture->getRtvHandle();
  m_command_buffer->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
  if (info.color_attachments[0].load_op == LoadOp::CLEAR) {
    float c[4] = {
        info.color_attachments[0].clear_value.r,
        info.color_attachments[0].clear_value.g,
        info.color_attachments[0].clear_value.b,
        info.color_attachments[0].clear_value.a,
    };
    m_command_buffer->ClearRenderTargetView(rtvHandle, c, 0, nullptr);
  }
}

void Dx12CommandList::endRendering() {
  // No-op: OMSetRenderTargets живёт до следующей смены
}

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

void Dx12CommandList::blitImage(RID src, RID dst, const ImageBlit &region) {}
} // namespace ssme::d3d12
