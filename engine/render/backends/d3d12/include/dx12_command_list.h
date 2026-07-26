#pragma once

#include "command_list.h"
#include "dx12_gpu_storage_fwd.h"
#include <wrl/client.h>
// Forward declaration
struct ID3D12GraphicsCommandList;
struct ID3D12CommandAllocator;

namespace ssme::d3d12 {

class Dx12Device;

class Dx12CommandList final : public CommandList {
public:
  Dx12CommandList(Dx12Device &device, Dx12GpuStorageMT &storage);
  ~Dx12CommandList() override = default;

  // --- Lifecycle ---
  void begin() override;
  void end() override;

  // --- Pipeline State ---
  void setGraphicsPipeline(RID pipeline_rid) override;
  void setComputePipeline(RID pipeline_rid) override {}

  void setViewport(const Viewport &viewport) override;
  void setScissor(const Rect &rect) override;
  void setDepthBias(float constant_factor, float slope_factor) override;

  // --- Resource Binding ---
  void setVertexBuffer(uint32_t first_binding, RID buffer_rid,
                       uint64_t offset) override;
  void setIndexBuffer(RID buffer_rid, uint64_t offset, IndexType type) override;
  void setDescriptorSet(uint32_t set_index, RID set_rid,
                        RID pipeline_rid) override;
  void setPushConstant(RID pipeline_rid, const UniformValue &value,
                       ShaderStageFlags stages, uint32_t offset = 0) override;

  // --- Drawing ---
  void draw(uint32_t vertex_count, uint32_t instance_count,
            uint32_t first_vertex, uint32_t first_instance) override;
  void drawIndexed(uint32_t index_count, uint32_t instance_count,
                   uint32_t first_index, int32_t vertex_offset,
                   uint32_t first_instance) override;
  void drawIndexedIndirect(RID buffer_rid, uint64_t offset, uint32_t draw_count,
                           uint32_t stride) override;

  // --- Compute ---
  void dispatch(uint32_t group_count_x, uint32_t group_count_y,
                uint32_t group_count_z) override;

  // --- Synchronization ---
  void pipelineBarrier(const BarrierInfo &barrier) override;

  // --- Render Pass Management ---
  void beginRendering(const RenderingInfo &info) override;
  void endRendering() override;

  // --- Resource Manipulation ---
  void copyBuffer(RID src, RID dst, const BufferCopy &region) override;
  void copyBufferToImage(RID src_buffer, RID dst_image,
                         const BufferImageCopy &region) override;
  void clearColorImage(RID image, const float color[4]) override;

  // --- DirectX-specific methods ---
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> const &
  getCommandListHandle() const & {
    return m_command_buffer;
  }
  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> const &
  getCommandAllocatorHandle() const & {
    return m_command_allocator;
  }

private:
  Dx12Device &m_device;
  Dx12GpuStorageMT &m_storage;
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_buffer = nullptr;
  // Получается, один allocator на несколько command list'ов в DX12 — это
  // ловушка: Reset() не сработает, пока GPU не дочитал **все** list'ы из этого
  // allocator. Поэтому Microsoft в туториалах и делают 1:1.
  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator = nullptr;
};

} // namespace ssme::d3d12
