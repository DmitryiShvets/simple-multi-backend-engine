#pragma once
#include "vulkan_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace ssme::vulkan {

class DescriptorSetLayout {
public:
  class Builder {
  public:
    Builder(VulkanDevice &device) : m_device(device) {}
    Builder &addBinding(uint32_t binding, vk::DescriptorType descriptorType,
                        vk::ShaderStageFlags stageFlags, uint32_t count = 1);

    std::unique_ptr<DescriptorSetLayout> build() const;

  private:
    VulkanDevice &m_device;
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_bindings{};
  };

  DescriptorSetLayout(
      VulkanDevice &device,
      std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings);
  ~DescriptorSetLayout();
  DescriptorSetLayout(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

  vk::DescriptorSetLayout getDescriptorSetLayout() const {
    return *m_ds_layout;
  }
  const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &
  getBindings() const {
    return m_bindings;
  }

private:
  VulkanDevice &m_device;
  vk::raii::DescriptorSetLayout m_ds_layout = nullptr;
  std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_bindings;

  friend class DescriptorWriter;
};

class DescriptorPool {
public:
  class Builder {
  public:
    Builder(VulkanDevice &device) : m_device{device} {}

    Builder &addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(vk::DescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<DescriptorPool> build() const;

  private:
    VulkanDevice &m_device;
    std::vector<vk::DescriptorPoolSize> m_pool_sizes{};
    uint32_t m_max_sets_count = 1000;
    vk::DescriptorPoolCreateFlags m_pool_flags;
  };

  DescriptorPool(VulkanDevice &device, uint32_t maxSets,
                 vk::DescriptorPoolCreateFlags poolFlags,
                 const std::vector<vk::DescriptorPoolSize> &poolSizes);
  ~DescriptorPool();
  DescriptorPool(const DescriptorPool &) = delete;
  DescriptorPool &operator=(const DescriptorPool &) = delete;

  vk::raii::DescriptorSet
  allocateDescriptor(const vk::DescriptorSetLayout &descriptorSetLayout) const;

  void freeDescriptors(std::vector<vk::raii::DescriptorSet> &descriptors) const;

  void resetPool();
  vk::DescriptorPool getDescriptorPool() { return *m_ds_pool; };

private:
  VulkanDevice &m_device;
  vk::raii::DescriptorPool m_ds_pool = nullptr;

  friend class DescriptorWriter;
};

class VulkanDescriptorSet {
public:
  VulkanDescriptorSet(VulkanDevice &device, vk::raii::DescriptorSet ds);
  ~VulkanDescriptorSet();

  // Non-copyable
  VulkanDescriptorSet(const VulkanDescriptorSet &) = delete;
  VulkanDescriptorSet &operator=(const VulkanDescriptorSet &) = delete;

  vk::DescriptorSet getHandle() const { return *m_decriptor_set; }

private:
  VulkanDevice &m_device;
  vk::raii::DescriptorSet m_decriptor_set = nullptr;
};

class DescriptorWriter {
public:
  DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

  DescriptorWriter &writeBuffer(uint32_t binding,
                                vk::DescriptorBufferInfo *bufferInfo);
  DescriptorWriter &writeImage(uint32_t binding,
                               vk::DescriptorImageInfo *imageInfo);

  std::unique_ptr<VulkanDescriptorSet> build();
  void overwrite(vk::raii::DescriptorSet &set);

private:
  DescriptorSetLayout &m_ds_layout;
  DescriptorPool &m_ds_pool;
  std::vector<vk::WriteDescriptorSet> m_writes;
};

} // namespace ssme::vulkan

// Справочник
// |-----|---------|-------|
// | `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER` | Uniform Buffer (UBO) | Константы для
// шейдера (матрицы, параметры) | | `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` |
// Storage Buffer (SSBO) | Чтение/запись больших данных | |
// `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` | Текстура + сэмплер |
// Изображения для шейдера | | `VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE` | Только
// текстура | Без сэмплера | | `VK_DESCRIPTOR_TYPE_SAMPLER` | Только сэмплер |
// Параметры фильтрации | | `VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT` | Subpass
// input | Для render pass | | `VK_DESCRIPTOR_TYPE_STORAGE_IMAGE` | Read/write
// image | Для compute шейдеров | | `VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER` |
// Uniform texel buffer | Структурированные данные | |
// `VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER` | Storage texel buffer | Read/write
// структурированные данные |
// ---
