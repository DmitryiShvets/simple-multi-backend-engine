#include "vulkan_command_list.h"
#include <stdexcept>

namespace { // Анонимное пространство имен, чтобы скрыть детали

// Вспомогательная функция для маппинга состояний RHI в Vulkan Layouts
VkImageLayout RhiStateToVkLayout(Render::ResourceState state) {
  switch (state) {
  case Render::ResourceState::UNDEFINED:
    return VK_IMAGE_LAYOUT_UNDEFINED;
  case Render::ResourceState::RENDER_TARGET:
    // Это состояние, в котором в изображение можно рисовать как в аттачмент
    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  case Render::ResourceState::PRESENT_SRC:
    // Это состояние, из которого изображение можно выводить на экран
    return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  case Render::ResourceState::TRANSFER_DST:
    // Это состояние для цели операции очистки или копирования
    return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  default:
    return VK_IMAGE_LAYOUT_UNDEFINED;
  }
}

// Вспомогательная структура для определения параметров барьера
struct BarrierInfo {
  VkAccessFlags access_mask;
  VkPipelineStageFlags stage_mask;
};

// Вспомогательная функция для маппинга состояний RHI в параметры доступа
BarrierInfo RhiStateToBarrierInfo(Render::ResourceState state) {
  switch (state) {
  case Render::ResourceState::UNDEFINED:
    return {0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
  case Render::ResourceState::RENDER_TARGET:
    return {VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  case Render::ResourceState::PRESENT_SRC:
    // Для этого состояния не требуется специфического доступа,
    // но синхронизация нужна на самом последнем этапе.
    return {0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
  case Render::ResourceState::TRANSFER_DST:
    return {VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT};
  default:
    return {0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
  }
}

} // end anonymous namespace

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
    throw std::runtime_error("failed to allocate command buffer!");
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
void VulkanCommandList::clearRenderTarget(RID renderTarget,
                                          const float color[4]) {
  VkImage image = m_resource_manager.get_val<VkImage>(renderTarget);

  VkClearColorValue clearColor;
  clearColor.float32[0] = color[0];
  clearColor.float32[1] = color[1];
  clearColor.float32[2] = color[2];
  clearColor.float32[3] = color[3];

  VkImageSubresourceRange subresourceRange{};
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.levelCount = 1;
  subresourceRange.layerCount = 1;

  // Эта команда требует, чтобы изображение было в layout TRANSFER_DST_OPTIMAL.
  // Ответственность за это лежит на том, кто вызывает эту команду.
  vkCmdClearColorImage(m_command_buffer, image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1,
                       &subresourceRange);
}

void VulkanCommandList::resourceBarrier(RID resource, ResourceState before,
                                        ResourceState after) {
  // 1. Получаем нативный хэндл VkImage из менеджера ресурсов
  VkImage image = m_resource_manager.get_val<VkImage>(resource);

  BarrierInfo before_info = RhiStateToBarrierInfo(before);
  BarrierInfo after_info = RhiStateToBarrierInfo(after);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = RhiStateToVkLayout(before);
  barrier.newLayout = RhiStateToVkLayout(after);
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = before_info.access_mask;
  barrier.dstAccessMask = after_info.access_mask;

  // 2. Записываем команду барьера в командный буфер
  vkCmdPipelineBarrier(
      m_command_buffer,
      before_info.stage_mask, // Этап, который должен завершиться ДО барьера
      after_info.stage_mask,  // Этап, который будет ждать ПОСЛЕ барьера
      0, 0, nullptr, 0, nullptr, 1, &barrier);
}
} // namespace Render::Vulkan
