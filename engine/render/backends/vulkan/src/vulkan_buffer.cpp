#include "vulkan_buffer.h"

// std
#include <cassert>
#include <cstring>

namespace ssme::vulkan {

VulkanBuffer::VulkanBuffer(VulkanDevice &device, vk::DeviceSize instanceSize,
                           uint64_t instanceCount,
                           vk::BufferUsageFlags usageFlags,
                           vk::MemoryPropertyFlags memoryPropertyFlags,
                           vk::DeviceSize minOffsetAlignment)
    : m_device(device), m_instance_size(instanceSize),
      m_instance_count(instanceCount), m_usage_flags(usageFlags),
      m_memory_property_flags(memoryPropertyFlags) {
  m_alignment_size = getAlignment(instanceSize, minOffsetAlignment);
  m_buffer_size = m_alignment_size * m_instance_count;
  auto [buffer, memory] = device.createBuffer(m_buffer_size, m_usage_flags,
                                              m_memory_property_flags);
  m_buffer = std::move(buffer);
  m_memory = std::move(memory);
}

VulkanBuffer::~VulkanBuffer() {
  // TODO: TEST IS MAPPING ORDER CORRECT IN APP
  if (m_mapped != nullptr) {
    unmap();
  }
}
/**
 * Returns the minimum instance size required to be compatible with devices
 * minOffsetAlignment
 *
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the
 * offset member (eg minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer mapping call
 */
vk::DeviceSize VulkanBuffer::getAlignment(vk::DeviceSize instanceSize,
                                          vk::DeviceSize minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the
 * specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to
 * map the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
void VulkanBuffer::map(vk::DeviceSize size, vk::DeviceSize offset) {
  assert(m_buffer != nullptr && m_memory != nullptr &&
         "Called map on buffer before create");
  m_mapped = m_memory.mapMemory(offset, size);
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void VulkanBuffer::unmap() {
  assert(m_mapped != nullptr && "Called unmap on unmapped buffer");

  if (m_mapped == nullptr) {
    return; // Protection in release build
  }

  m_memory.unmapMemory();
  m_mapped = nullptr;
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole
 * buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void VulkanBuffer::writeToBuffer(void *data, vk::DeviceSize size,
                                 vk::DeviceSize offset) {
  assert(m_mapped && "Cannot copy to unmapped buffer");

  if (size == vk::WholeSize) {
    memcpy(m_mapped, data, m_buffer_size);
  } else {
    char *memOffset = (char *)m_mapped;
    memOffset += offset;
    memcpy(memOffset, data, size);
  }
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE
 * to flush the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
void VulkanBuffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
  vk::MappedMemoryRange mapped_range{
      .memory = m_memory, .offset = offset, .size = size};
  m_device.getHandle().flushMappedMemoryRanges(mapped_range);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass
 * VK_WHOLE_SIZE to invalidate the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
void VulkanBuffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
  vk::MappedMemoryRange mapped_range{
      .memory = m_memory, .offset = offset, .size = size};
  m_device.getHandle().invalidateMappedMemoryRanges(mapped_range);
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
vk::DescriptorBufferInfo
VulkanBuffer::getDescriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) {
  return vk::DescriptorBufferInfo{
      .buffer = m_buffer,
      .offset = offset,
      .range = size,
  };
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of
 * index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void VulkanBuffer::writeToIndex(void *data, int index) {
  writeToBuffer(data, m_instance_size, index * m_alignment_size);
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it
 * visible to the device
 *
 * @param index Used in offset calculation
 *
 */
void VulkanBuffer::flushIndex(int index) {
  flush(m_alignment_size, index * m_alignment_size);
}

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
vk::DescriptorBufferInfo VulkanBuffer::getDescriptorInfoForIndex(int index) {
  return getDescriptorInfo(m_alignment_size, index * m_alignment_size);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return VkResult of the invalidate call
 */
void VulkanBuffer::invalidateIndex(int index) {
  invalidate(m_alignment_size, index * m_alignment_size);
}

} // namespace ssme::vulkan
