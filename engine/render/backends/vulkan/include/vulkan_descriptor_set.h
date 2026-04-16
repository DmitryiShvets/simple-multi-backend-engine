#pragma once
#include "vulkan_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace ssme::vulkan {

class VulkanDescriptorSetLayout {
public:
  class Builder {
  public:
    Builder(VulkanDevice &device) : m_device(device) {}
    Builder &addBinding(uint32_t binding, vk::DescriptorType descriptorType,
                        vk::ShaderStageFlags stageFlags, uint32_t count = 1);

    std::unique_ptr<VulkanDescriptorSetLayout> build() const;

  private:
    VulkanDevice &m_device;
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_bindings{};
  };

  VulkanDescriptorSetLayout(
      VulkanDevice &device,
      std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings);
  ~VulkanDescriptorSetLayout();
  VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &) = delete;
  VulkanDescriptorSetLayout &operator=(const VulkanDescriptorSetLayout &) = delete;

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

class VulkanDescriptorPool {
public:
  class Builder {
  public:
    Builder(VulkanDevice &device) : m_device{device} {}

    Builder &addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(vk::DescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<VulkanDescriptorPool> build() const;

  private:
    VulkanDevice &m_device;
    std::vector<vk::DescriptorPoolSize> m_pool_sizes{};
    uint32_t m_max_sets_count = 1000;
    vk::DescriptorPoolCreateFlags m_pool_flags;
  };

  VulkanDescriptorPool(VulkanDevice &device, uint32_t maxSets,
                 vk::DescriptorPoolCreateFlags poolFlags,
                 const std::vector<vk::DescriptorPoolSize> &poolSizes);
  ~VulkanDescriptorPool();
  VulkanDescriptorPool(const VulkanDescriptorPool &) = delete;
  VulkanDescriptorPool &operator=(const VulkanDescriptorPool &) = delete;

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
  DescriptorWriter(VulkanDescriptorSetLayout &setLayout, VulkanDescriptorPool &pool);

  DescriptorWriter &writeBuffer(uint32_t binding,
                                vk::DescriptorBufferInfo *bufferInfo);
  DescriptorWriter &writeImage(uint32_t binding,
                               vk::DescriptorImageInfo *imageInfo);

  std::unique_ptr<VulkanDescriptorSet> build();
  void overwrite(vk::raii::DescriptorSet &set);

private:
  VulkanDescriptorSetLayout &m_ds_layout;
  VulkanDescriptorPool &m_ds_pool;
  std::vector<vk::WriteDescriptorSet> m_writes;
};

} // namespace ssme::vulkan

// Reference guide
// |-----|---------|-------|
// | `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER` | Uniform Buffer (UBO) | Shader constants (matrices, parameters) | | `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` |
// Storage Buffer (SSBO) | Read/write large data | |
// `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` | Texture + sampler |
// Images for shader | | `VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE` | Texture only
// | Without sampler | | `VK_DESCRIPTOR_TYPE_SAMPLER` | Sampler only |
// Filtering parameters | | `VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT` | Subpass
// input | For render pass | | `VK_DESCRIPTOR_TYPE_STORAGE_IMAGE` | Read/write
// image | For compute shaders | | `VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER` |
// Uniform texel buffer | Structured data | |
// `VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER` | Storage texel buffer | Read/write
// structured data |
// ---
