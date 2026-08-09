
#include "vulkan_command_list.h"
#include "utils/debug_assert.h"
#include "vulkan/vulkan.hpp"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_device.h"
#include "vulkan_gpu_storage.h"
#include "vulkan_helpers.h"
#include "vulkan_pipeline.h"
#include "vulkan_texture.h"
#include <cassert>
#include <cstdint>

namespace ssme::vulkan {

VulkanCommandList::VulkanCommandList(VulkanDevice &vkDevice,
                                     VulkanGpuStorageMT &storage)
    : m_device(vkDevice), m_storage(storage) {
  m_command_buffer = m_device.createCommandBuffer();
}

VulkanCommandList::~VulkanCommandList() {}

void VulkanCommandList::begin() { m_command_buffer.begin({}); }
void VulkanCommandList::end() { m_command_buffer.end(); }

void VulkanCommandList::setGraphicsPipeline(RID pipeline_rid) {
  auto pipeline = m_storage.get<VulkanPipeLine>(pipeline_rid);
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
  auto buffer = m_storage.get<VulkanBuffer>(buffer_rid);
  if (buffer) {
    vk::Buffer vk_buffer = buffer->getBuffer();
    m_command_buffer.bindVertexBuffers(first_binding, vk_buffer, offset);
  }
}
void VulkanCommandList::setIndexBuffer(RID buffer_rid, uint64_t offset,
                                       IndexType type) {
  auto buffer = m_storage.get<VulkanBuffer>(buffer_rid);
  if (buffer) {
    vk::Buffer vk_buffer = buffer->getBuffer();
    m_command_buffer.bindIndexBuffer(vk_buffer, offset, toVkIndexType(type));
  }
}
void VulkanCommandList::setDescriptorSet(uint32_t set_index, RID set_rid,
                                         RID pipeline_rid) {
  auto descriptor_set = m_storage.get<VulkanDescriptorSet>(set_rid);
  auto pipeline = m_storage.get<VulkanPipeLine>(pipeline_rid);
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
  auto pipeline = m_storage.get<VulkanPipeLine>(pipeline_rid);
  debug_assert(pipeline != nullptr,
               "Internal error! check creation of pipeline");
  auto *data = static_cast<const uint8_t *>(value.data());
  m_command_buffer.pushConstants<uint8_t>(
      pipeline->getLayoutHandle(), toVkShaderStageFlags(stages), offset,
      vk::ArrayProxy<const uint8_t>(static_cast<uint32_t>(value.size()), data));
}
void VulkanCommandList::draw(uint32_t vertex_count, uint32_t instance_count,
                             uint32_t first_vertex, uint32_t first_instance) {
  m_command_buffer.draw(vertex_count, instance_count, first_vertex,
                        first_instance);
}
void VulkanCommandList::drawIndexed(uint32_t index_count,
                                    uint32_t instance_count,
                                    uint32_t first_index, int32_t vertex_offset,
                                    uint32_t first_instance) {
  m_command_buffer.drawIndexed(index_count, instance_count, first_index,
                               vertex_offset, first_instance);
}
void VulkanCommandList::drawIndexedIndirect(RID buffer_rid, uint64_t offset,
                                            uint32_t draw_count,
                                            uint32_t stride) {}
void VulkanCommandList::dispatch(uint32_t group_count_x, uint32_t group_count_y,
                                 uint32_t group_count_z) {}

void VulkanCommandList::pipelineBarrier(const BarrierInfo &barrier) {
  for (const auto &img_barrier_desc : barrier.image_barriers) {
    auto texture = m_storage.get<VulkanTexture>(img_barrier_desc.image);
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
  for (const auto &a : info.color_attachments) {
    auto tex = m_storage.get<VulkanTexture>(a.texture);
    if (!tex)
      continue;
    vk::AttachmentLoadOp load = vk::AttachmentLoadOp::eClear;
    if (a.load_op == LoadOp::LOAD)
      load = vk::AttachmentLoadOp::eLoad;
    else if (a.load_op == LoadOp::DONT_CARE)
      load = vk::AttachmentLoadOp::eDontCare;
    vk::AttachmentStoreOp store = vk::AttachmentStoreOp::eStore;
    if (a.store_op == StoreOp::DONT_CARE)
      store = vk::AttachmentStoreOp::eDontCare;
    color_attachments.push_back({.imageView = tex->getImageView(),
                                 .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                                 .loadOp = load,
                                 .storeOp = store,
                                 .clearValue = {{a.clear_value.r, a.clear_value.g,
                                                 a.clear_value.b, a.clear_value.a}}});
  }

  bool has_depth = info.depth_attachment.texture.isValid();
  vk::RenderingAttachmentInfo depth_attachment;
  if (has_depth) {
    auto dtex = m_storage.get<VulkanTexture>(info.depth_attachment.texture);
    if (!dtex)
      has_depth = false;
    else {
      vk::AttachmentLoadOp load = vk::AttachmentLoadOp::eClear;
      if (info.depth_attachment.load_op == LoadOp::LOAD)
        load = vk::AttachmentLoadOp::eLoad;
      else if (info.depth_attachment.load_op == LoadOp::DONT_CARE)
        load = vk::AttachmentLoadOp::eDontCare;
      vk::AttachmentStoreOp store = vk::AttachmentStoreOp::eStore;
      if (info.depth_attachment.store_op == StoreOp::DONT_CARE)
        store = vk::AttachmentStoreOp::eDontCare;
      depth_attachment = {.imageView = dtex->getImageView(),
                          .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                          .loadOp = load,
                          .storeOp = store,
                          .clearValue = {{info.depth_attachment.clear_value, 0}}};
    }
  }

  vk::Rect2D render_area = {
      {info.render_area.x, info.render_area.y},
      {info.render_area.width, info.render_area.height},
  };
  vk::RenderingInfo rendering_info{
      .renderArea = render_area,
      .layerCount = 1,
      .colorAttachmentCount = static_cast<uint32_t>(color_attachments.size()),
      .pColorAttachments = color_attachments.data(),
      .pDepthAttachment = has_depth ? &depth_attachment : nullptr,
  };
  m_command_buffer.beginRendering(rendering_info);
}

void VulkanCommandList::endRendering() { m_command_buffer.endRendering(); }
void VulkanCommandList::copyBuffer(RID src, RID dst, const BufferCopy &region) {
}
void VulkanCommandList::copyBufferToImage(RID src_buffer, RID dst_image,
                                          const BufferImageCopy &region) {}
void VulkanCommandList::clearColorImage(RID image, const float color[4]) {}

void VulkanCommandList::blitImage(RID src, RID dst, const ImageBlit &region) {
  auto src_tex = m_storage.get<VulkanTexture>(src);
  auto dst_tex = m_storage.get<VulkanTexture>(dst);
  if (!src_tex || !dst_tex) {
    assert(false && "blitImage: texture not found");
    return;
  }
  vk::ImageBlit blit;
  blit.srcSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
  blit.dstSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};

  blit.srcOffsets[0] = vk::Offset3D{.x = region.src_x, .y = region.src_y, .z = 0};
  blit.srcOffsets[1] = vk::Offset3D{
      .x = region.src_x + (int32_t)region.width,
      .y = region.src_y + (int32_t)region.height,
      .z = 1};
  blit.dstOffsets[0] = vk::Offset3D{.x = region.dst_x, .y = region.dst_y, .z = 0};
  blit.dstOffsets[1] = vk::Offset3D{
      .x = region.dst_x + (int32_t)region.width,
      .y = region.dst_y + (int32_t)region.height,
      .z = 1};

  m_command_buffer.blitImage(
      src_tex->getImage(), vk::ImageLayout::eTransferSrcOptimal,
      dst_tex->getImage(), vk::ImageLayout::eTransferDstOptimal, blit,
      vk::Filter::eLinear);
}


} // namespace ssme::vulkan
