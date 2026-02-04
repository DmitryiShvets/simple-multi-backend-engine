#include "vulkan_command_list.h"
#include "vulkan_device.h" // Include full definition to resolve incomplete type errors
#include <stdexcept>

namespace Render::Vulkan {

VulkanCommandList::VulkanCommandList(VulkanDevice &vkDevice,
                                     VulkanResourceManager &vkResourceManager)
    : m_device(vkDevice), m_resource_manager(vkResourceManager),
      m_command_buffer(VK_NULL_HANDLE) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = m_device.getCommandPool();
  allocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(m_device.getDeviceHandle(), &allocInfo,
                               &m_command_buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

VulkanCommandList::~VulkanCommandList() {
  if (m_command_buffer != VK_NULL_HANDLE) {
    vkFreeCommandBuffers(m_device.getDeviceHandle(), m_device.getCommandPool(),
                         1, &m_command_buffer);
  }
}

void VulkanCommandList::begin() {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if (vkBeginCommandBuffer(m_command_buffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
}
void VulkanCommandList::end() {
  if (vkEndCommandBuffer(m_command_buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

// --- Stub implementations for the new RHI interface ---

void VulkanCommandList::setGraphicsPipeline(RID pipeline_rid) {}
void VulkanCommandList::setComputePipeline(RID pipeline_rid) {}
void VulkanCommandList::setViewport(const Viewport& viewport) {}
void VulkanCommandList::setScissor(const Rect& rect) {}
void VulkanCommandList::setDepthBias(float constant_factor, float slope_factor) {}
void VulkanCommandList::setVertexBuffer(uint32_t first_binding, RID buffer_rid, uint64_t offset) {}
void VulkanCommandList::setIndexBuffer(RID buffer_rid, uint64_t offset, IndexType type) {}
void VulkanCommandList::setDescriptorSet(uint32_t set_index, RID set_rid) {}
void VulkanCommandList::setPushConstant(RID pipeline_layout_rid, ShaderStageFlags stages, const void* data, uint32_t size, uint32_t offset) {}
void VulkanCommandList::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {}
void VulkanCommandList::drawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) {}
void VulkanCommandList::drawIndexedIndirect(RID buffer_rid, uint64_t offset, uint32_t draw_count, uint32_t stride) {}
void VulkanCommandList::dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) {}
void VulkanCommandList::pipelineBarrier(const BarrierInfo& barrier) {}
void VulkanCommandList::beginRendering(const RenderingInfo& info) {}
void VulkanCommandList::endRendering() {}
void VulkanCommandList::copyBuffer(RID src, RID dst, const BufferCopy& region) {}
void VulkanCommandList::copyBufferToImage(RID src_buffer, RID dst_image, const BufferImageCopy& region) {}
void VulkanCommandList::clearColorImage(RID image, const float color[4]) {}

} // namespace Render::Vulkan
