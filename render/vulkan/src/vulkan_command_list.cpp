
#include "vulkan_command_list.h"
#include "vulkan_buffer.h"
#include "vulkan_device.h"
#include "vulkan_pipeline.h"
#include "vulkan_texture.h"
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

void VulkanCommandList::setGraphicsPipeline(RID pipeline_rid) {
  auto pipeline = m_resource_manager.get_ptr<VulkanPipeLine>(pipeline_rid);
  if (pipeline) {
    vkCmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->getHandle());
  }
}
void VulkanCommandList::setComputePipeline(RID pipeline_rid) {}

void VulkanCommandList::setViewport(const Viewport &viewport) {
  VkViewport vk_viewport = {
      .x = viewport.x,
      .y = viewport.y,
      .width = viewport.width,
      .height = viewport.height,
      .minDepth = viewport.minDepth,
      .maxDepth = viewport.maxDepth,
  };
  vkCmdSetViewport(m_command_buffer, 0, 1, &vk_viewport);
}

void VulkanCommandList::setScissor(const Rect &rect) {
  VkRect2D vk_rect = {
      .offset = {rect.x, rect.y},
      .extent = {rect.width, rect.height},
  };
  vkCmdSetScissor(m_command_buffer, 0, 1, &vk_rect);
}

void VulkanCommandList::setDepthBias(float constant_factor,
                                     float slope_factor) {}
void VulkanCommandList::setVertexBuffer(uint32_t first_binding, RID buffer_rid,
                                        uint64_t offset) {
  auto buffer = m_resource_manager.get_ptr<VulkanDataBuffer>(buffer_rid);
  if (buffer) {
    VkBuffer vk_buffer = buffer->getBuffer();
    vkCmdBindVertexBuffers(m_command_buffer, first_binding, 1, &vk_buffer,
                           &offset);
  }
}
void VulkanCommandList::setIndexBuffer(RID buffer_rid, uint64_t offset,
                                       IndexType type) {}
void VulkanCommandList::setDescriptorSet(uint32_t set_index, RID set_rid) {}
void VulkanCommandList::setPushConstant(RID pipeline_layout_rid,
                                        ShaderStageFlags stages,
                                        const void *data, uint32_t size,
                                        uint32_t offset) {}
void VulkanCommandList::draw(uint32_t vertex_count, uint32_t instance_count,
                             uint32_t first_vertex, uint32_t first_instance) {
  vkCmdDraw(m_command_buffer, vertex_count, instance_count, first_vertex,
            first_instance);
}
void VulkanCommandList::drawIndexed(uint32_t index_count,
                                    uint32_t instance_count,
                                    uint32_t first_index, int32_t vertex_offset,
                                    uint32_t first_instance) {}
void VulkanCommandList::drawIndexedIndirect(RID buffer_rid, uint64_t offset,
                                            uint32_t draw_count,
                                            uint32_t stride) {}
void VulkanCommandList::dispatch(uint32_t group_count_x, uint32_t group_count_y,
                                 uint32_t group_count_z) {}
// Helper
VkImageLayout ImageLayout_to_VkImageLayout(ImageLayout layout) {
  switch (layout) {
  case ImageLayout::UNDEFINED:
    return VK_IMAGE_LAYOUT_UNDEFINED;
  case ImageLayout::COLOR_ATTACHMENT:
    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  case ImageLayout::PRESENT_SRC:
    return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  case ImageLayout::TRANSFER_DST:
    return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  case ImageLayout::SHADER_READ_ONLY:
    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  default:
    return VK_IMAGE_LAYOUT_UNDEFINED;
  }
}

// Helper to set barrier masks and stages based on layout transition
void set_barrier_masks(VkImageMemoryBarrier &barrier, VkImageLayout old_layout,
                       VkImageLayout new_layout,
                       VkPipelineStageFlags &src_stages,
                       VkPipelineStageFlags &dst_stages) {
  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    src_stages |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dst_stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    src_stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dst_stages |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }
}

void VulkanCommandList::pipelineBarrier(const BarrierInfo &barrier) {
  std::vector<VkImageMemoryBarrier> image_barriers;
  VkPipelineStageFlags src_stages = 0;
  VkPipelineStageFlags dst_stages = 0;

  for (const auto &img_barrier_desc : barrier.image_barriers) {
    auto texture =
        m_resource_manager.get_ptr<VulkanTexture>(img_barrier_desc.image);
    if (!texture)
      continue;

    VkImageMemoryBarrier vk_barrier{};
    vk_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vk_barrier.oldLayout =
        ImageLayout_to_VkImageLayout(img_barrier_desc.old_layout);
    vk_barrier.newLayout =
        ImageLayout_to_VkImageLayout(img_barrier_desc.new_layout);
    vk_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vk_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vk_barrier.image = texture->getImage();
    vk_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_barrier.subresourceRange.baseMipLevel = 0;
    vk_barrier.subresourceRange.levelCount = 1;
    vk_barrier.subresourceRange.baseArrayLayer = 0;
    vk_barrier.subresourceRange.layerCount = 1;

    set_barrier_masks(vk_barrier, vk_barrier.oldLayout, vk_barrier.newLayout,
                      src_stages, dst_stages);
    image_barriers.push_back(vk_barrier);
  }

  if (!image_barriers.empty()) {
    vkCmdPipelineBarrier(
        m_command_buffer, src_stages, dst_stages, 0, 0, nullptr, 0, nullptr,
        static_cast<uint32_t>(image_barriers.size()), image_barriers.data());
  }
}

void VulkanCommandList::beginRendering(const RenderingInfo &info) {
  std::vector<VkRenderingAttachmentInfo> color_attachments;
  for (const auto &attachment_info : info.color_attachments) {
    VkRenderingAttachmentInfo vk_attachment_info{};
    vk_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    auto back_buffer_texture =
        m_resource_manager.get_ptr<VulkanTexture>(attachment_info.texture);
    vk_attachment_info.imageView = back_buffer_texture->getImageView();
    vk_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vk_attachment_info.loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR; // TODO: Convert from abstract op
    vk_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO: Convert
    vk_attachment_info.clearValue.color = {
        {attachment_info.clear_value.r, attachment_info.clear_value.g,
         attachment_info.clear_value.b, attachment_info.clear_value.a}};
    color_attachments.push_back(vk_attachment_info);
  }

  // TODO: Get render area from somewhere
  VkRect2D render_area = {
      {.x = info.render_area.x, .y = info.render_area.y},
      {.width = info.render_area.width, .height = info.render_area.height}};

  VkRenderingInfo vk_rendering_info{};
  vk_rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  vk_rendering_info.renderArea = render_area;
  vk_rendering_info.layerCount = 1;
  vk_rendering_info.colorAttachmentCount =
      static_cast<uint32_t>(color_attachments.size());
  vk_rendering_info.pColorAttachments = color_attachments.data();

  m_device.pfn_vkCmdBeginRenderingKHR(m_command_buffer, &vk_rendering_info);
}
void VulkanCommandList::endRendering() {
  m_device.pfn_vkCmdEndRenderingKHR(m_command_buffer);
}
void VulkanCommandList::copyBuffer(RID src, RID dst, const BufferCopy &region) {
}
void VulkanCommandList::copyBufferToImage(RID src_buffer, RID dst_image,
                                          const BufferImageCopy &region) {}
void VulkanCommandList::clearColorImage(RID image, const float color[4]) {}

} // namespace Render::Vulkan
