#pragma once
#include "vulkan_device.h"
#include <cstdint>

namespace ssme::vulkan {

class VulkanDataBuffer {
public:
  VulkanDataBuffer(VulkanDevice &device, vk::DeviceSize instanceSize,
                   uint32_t stride, uint32_t instanceCount,
                   vk::BufferUsageFlags usageFlags,
                   vk::MemoryPropertyFlags memoryPropertyFlags,
                   vk::DeviceSize minOffsetAlignment = 1);
  ~VulkanDataBuffer();

  VulkanDataBuffer(const VulkanDataBuffer &) = delete;
  VulkanDataBuffer &operator=(const VulkanDataBuffer &) = delete;

  void map(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
  void unmap();
  void writeToBuffer(void *data, vk::DeviceSize size = vk::WholeSize,
                     vk::DeviceSize offset = 0);
  void flush(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
  void invalidate(vk::DeviceSize size = vk::WholeSize,
                  vk::DeviceSize offset = 0);
  uint32_t count() const { return m_buffer_size / m_stride; }
  void writeToIndex(void *data, int index);
  void flushIndex(int index);
  void invalidateIndex(int index);
  vk::DescriptorBufferInfo
  getDescriptorInfo(vk::DeviceSize size = vk::WholeSize,
                    vk::DeviceSize offset = 0);
  vk::DescriptorBufferInfo getDescriptorInfoForIndex(int index);

  vk::Buffer getBuffer() const { return *m_buffer; }
  void *getMappedMemory() const { return m_mapped; }
  uint32_t getInstanceCount() const { return m_instance_count; }
  vk::DeviceSize getInstanceSize() const { return m_instance_size; }
  vk::DeviceSize getAlignmentSize() const { return m_alignment_size; }
  vk::BufferUsageFlags getUsageFlags() const { return m_usage_flags; }
  vk::MemoryPropertyFlags getMemoryPropertyFlags() const {
    return m_memory_property_flags;
  }

  vk::DeviceSize getBufferSize() const { return m_buffer_size; }

private:
  static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize,
                                     vk::DeviceSize minOffsetAlignment);

  VulkanDevice &m_device;
  void *m_mapped = nullptr;
  vk::raii::Buffer m_buffer = nullptr;
  vk::raii::DeviceMemory m_memory = nullptr;

  vk::DeviceSize m_buffer_size;
  vk::DeviceSize m_instance_size;
  vk::DeviceSize m_alignment_size;
  uint32_t m_instance_count;
  uint32_t m_stride;
  vk::BufferUsageFlags m_usage_flags;
  vk::MemoryPropertyFlags m_memory_property_flags;
};

} // namespace ssme::vulkan
