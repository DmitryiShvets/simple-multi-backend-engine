
#include "vulkan_command_list.h"
#include "vulkan/vulkan.hpp"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include "vulkan_pipeline.h"
#include "vulkan_resource_manager.h"
#include "vulkan_texture.h"
#include <cassert>

namespace Render::Vulkan {

VulkanCommandList::VulkanCommandList(VulkanDevice &vkDevice,
                                     VulkanResourceManager &vkResourceManager)
    : m_device(vkDevice), m_resource_manager(vkResourceManager) {
  m_command_buffer = m_device.createCommandBuffer();
}

VulkanCommandList::~VulkanCommandList() {}

void VulkanCommandList::begin() { m_command_buffer.begin({}); }
void VulkanCommandList::end() { m_command_buffer.end(); }

void VulkanCommandList::setGraphicsPipeline(RID pipeline_rid) {
  auto pipeline = m_resource_manager.get_ptr<VulkanPipeLine>(pipeline_rid);
  if (pipeline) {
    m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                   pipeline->getHandle());
  }
}
void VulkanCommandList::setComputePipeline(RID pipeline_rid) {}

void VulkanCommandList::setViewport(const Viewport &viewport) {
  vk::Viewport vk_viewport = {
      .x = viewport.x,
      .y = viewport.y,
      .width = viewport.width,
      .height = viewport.height,
      .minDepth = viewport.minDepth,
      .maxDepth = viewport.maxDepth,
  };
  m_command_buffer.setViewport(0, vk_viewport);
}

void VulkanCommandList::setScissor(const Rect &rect) {
  vk::Rect2D vk_rect = {
      .offset = {rect.x, rect.y},
      .extent = {rect.width, rect.height},
  };
  m_command_buffer.setScissor(0, vk_rect);
}

void VulkanCommandList::setDepthBias(float constant_factor,
                                     float slope_factor) {}
void VulkanCommandList::setVertexBuffer(uint32_t first_binding, RID buffer_rid,
                                        uint64_t offset) {
  auto buffer = m_resource_manager.get_ptr<VulkanDataBuffer>(buffer_rid);
  if (buffer) {
    vk::Buffer vk_buffer = buffer->getBuffer();
    m_command_buffer.bindVertexBuffers(first_binding, vk_buffer, offset);
  }
}
void VulkanCommandList::setIndexBuffer(RID buffer_rid, uint64_t offset,
                                       IndexType type) {}
void VulkanCommandList::setDescriptorSet(uint32_t set_index, RID set_rid,
                                         RID pipeline_rid) {
  auto descriptor_set =
      m_resource_manager.get_ptr<VulkanDescriptorSet>(set_rid);
  auto pipeline = m_resource_manager.get_ptr<VulkanPipeLine>(pipeline_rid);
  if (descriptor_set && pipeline) {
    m_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                         pipeline->getLayoutHandle(), set_index,
                                         descriptor_set->getHandle(), nullptr);
  }
}
void VulkanCommandList::setPushConstant(RID pipeline_rid,
                                        const UniformValue &value,
                                        ShaderStageFlags stages,
                                        uint32_t offset) {
  auto pipeline = m_resource_manager.get_ptr<VulkanPipeLine>(pipeline_rid);
  auto *data = static_cast<const uint8_t *>(value.data());
  m_command_buffer.pushConstants<uint8_t>(
      pipeline->getLayoutHandle(), toVkShaderStageFlags(stages), offset,
      vk::ArrayProxy<const uint8_t>(value.size(), data));
}
void VulkanCommandList::draw(uint32_t vertex_count, uint32_t instance_count,
                             uint32_t first_vertex, uint32_t first_instance) {
  m_command_buffer.draw(vertex_count, instance_count, first_vertex,
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

void VulkanCommandList::pipelineBarrier(const BarrierInfo &barrier) {
  for (const auto &img_barrier_desc : barrier.image_barriers) {
    auto texture =
        m_resource_manager.get_ptr<VulkanTexture>(img_barrier_desc.image);
    if (!texture) {
      assert(false && "Texture is not found");
      continue;
    }
    auto old_layout = toVkImageLayout(img_barrier_desc.old_layout);
    auto new_layout = toVkImageLayout(img_barrier_desc.new_layout);
    auto masks = getImageMemoryBarrierMasks(old_layout, new_layout);
    m_device.pipelineBarrier(m_command_buffer, texture->getImage(),
                             texture->getFormat(), old_layout, new_layout,
                             masks.src_mask, masks.dst_mask, masks.src_stages,
                             masks.dst_stages);
  }
}

void VulkanCommandList::beginRendering(const RenderingInfo &info) {

  std::vector<vk::RenderingAttachmentInfo> color_attachments;
  for (const auto &attachment_info : info.color_attachments) {
    auto back_buffer_texture =
        m_resource_manager.get_ptr<VulkanTexture>(attachment_info.texture);
    vk::RenderingAttachmentInfo vk_attachment_info{
        .imageView = back_buffer_texture->getImageView(),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue =
            {
                {attachment_info.clear_value.r, attachment_info.clear_value.g,
                 attachment_info.clear_value.b, attachment_info.clear_value.a},
            },
    };
    color_attachments.push_back(vk_attachment_info);
  }

  auto back_depth_buffer_texture =
      m_resource_manager.get_ptr<VulkanTexture>(info.depth_attachment.texture);
  vk::RenderingAttachmentInfo depth_attachment{
      .imageView = back_depth_buffer_texture->getImageView(),
      .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .clearValue = {{1.0f, 0}},
  };
  // TODO: Get render area from somewhere
  vk::Rect2D render_area = {
      {.x = info.render_area.x, .y = info.render_area.y},
      {.width = info.render_area.width, .height = info.render_area.height},
  };

  vk::RenderingInfo rendering_info{
      .renderArea = render_area,
      .layerCount = 1,
      .colorAttachmentCount = static_cast<uint32_t>(color_attachments.size()),
      .pColorAttachments = color_attachments.data(),
      .pDepthAttachment = &depth_attachment,
  };
  m_command_buffer.beginRendering(rendering_info);
}
void VulkanCommandList::endRendering() {
  m_command_buffer.endRendering();
}
void VulkanCommandList::copyBuffer(RID src, RID dst, const BufferCopy &region) {
}
void VulkanCommandList::copyBufferToImage(RID src_buffer, RID dst_image,
                                          const BufferImageCopy &region) {}
void VulkanCommandList::clearColorImage(RID image, const float color[4]) {}

} // namespace Render::Vulkan
