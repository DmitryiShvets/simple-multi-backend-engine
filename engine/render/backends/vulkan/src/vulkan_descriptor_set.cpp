#include "vulkan_descriptor_set.h"

// std
#include <cassert>
#include <memory>
#include <utility>

namespace ssme::vulkan {

// *************** Descriptor Set Layout Builder *********************

VulkanDescriptorSetLayout::Builder &VulkanDescriptorSetLayout::Builder::addBinding(
    uint32_t binding, vk::DescriptorType descriptorType,
    vk::ShaderStageFlags stageFlags, uint32_t count) {
  assert(m_bindings.count(binding) == 0 && "Binding already in use");
  vk::DescriptorSetLayoutBinding new_binding{
      .binding = binding,
      .descriptorType = descriptorType,
      .descriptorCount = count,
      .stageFlags = stageFlags,
  };
  m_bindings[binding] = new_binding;
  return *this;
}

std::unique_ptr<VulkanDescriptorSetLayout>
VulkanDescriptorSetLayout::Builder::build() const {
  return std::make_unique<VulkanDescriptorSetLayout>(m_device, m_bindings);
}

// *************** Descriptor Set Layout *********************

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
    VulkanDevice &device,
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings)
    : m_device(device), m_bindings(bindings) {
  std::vector<vk::DescriptorSetLayoutBinding> bindings_array{};
  for (auto &kv : bindings) {
    bindings_array.push_back(kv.second);
  }

  vk::DescriptorSetLayoutCreateInfo create_info{
      .bindingCount = static_cast<uint32_t>(bindings_array.size()),
      .pBindings = bindings_array.data(),
  };
  m_ds_layout =
      vk::raii::DescriptorSetLayout(m_device.getHandle(), create_info);
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
}

// *************** Descriptor Pool Builder *********************

VulkanDescriptorPool::Builder &
VulkanDescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType,
                                     uint32_t count) {
  m_pool_sizes.push_back(vk::DescriptorPoolSize{descriptorType, count});
  return *this;
}

VulkanDescriptorPool::Builder &
VulkanDescriptorPool::Builder::setPoolFlags(vk::DescriptorPoolCreateFlags flags) {
  m_pool_flags = flags;
  return *this;
}
VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::setMaxSets(uint32_t count) {
  m_max_sets_count = count;
  return *this;
}

std::unique_ptr<VulkanDescriptorPool> VulkanDescriptorPool::Builder::build() const {
  return std::make_unique<VulkanDescriptorPool>(m_device, m_max_sets_count,
                                          m_pool_flags, m_pool_sizes);
}

// *************** Descriptor Pool *********************

VulkanDescriptorPool::VulkanDescriptorPool(
    VulkanDevice &device, uint32_t maxSets,
    vk::DescriptorPoolCreateFlags poolFlags,
    const std::vector<vk::DescriptorPoolSize> &poolSizes)
    : m_device(device) {
  vk::DescriptorPoolCreateInfo creat_info{
      .flags = poolFlags,
      .maxSets = maxSets,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data(),
  };
  m_ds_pool = vk::raii::DescriptorPool(m_device.getHandle(), creat_info);
}

VulkanDescriptorPool::~VulkanDescriptorPool() {

}

vk::raii::DescriptorSet VulkanDescriptorPool::allocateDescriptor(
    const vk::DescriptorSetLayout &descriptorSetLayout) const {
  vk::DescriptorSetAllocateInfo alloc_info{
      .descriptorPool = m_ds_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout,
  };

  vk::raii::DescriptorSet ds =
      std::move(m_device.getHandle().allocateDescriptorSets(alloc_info).at(0));
  return ds;
}

void VulkanDescriptorPool::freeDescriptors(
    std::vector<vk::raii::DescriptorSet> &descriptors) const {
  for (auto &ds : descriptors) {
    ds.clear();
  }
}

void VulkanDescriptorPool::resetPool() {
  m_ds_pool.reset();
}

// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(VulkanDescriptorSetLayout &setLayout,
                                   VulkanDescriptorPool &pool)
    : m_ds_layout(setLayout), m_ds_pool(pool) {}

DescriptorWriter &
DescriptorWriter::writeBuffer(uint32_t binding,
                              vk::DescriptorBufferInfo *bufferInfo) {
  assert(m_ds_layout.m_bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &binding_desc = m_ds_layout.m_bindings[binding];

  assert(binding_desc.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  vk::WriteDescriptorSet write{
      .dstBinding = binding,
      .descriptorCount = 1,
      .descriptorType = binding_desc.descriptorType,
      .pBufferInfo = bufferInfo,
  };

  m_writes.push_back(write);
  return *this;
}

DescriptorWriter &
DescriptorWriter::writeImage(uint32_t binding,
                             vk::DescriptorImageInfo *imageInfo) {
  assert(m_ds_layout.m_bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &binding_desc = m_ds_layout.m_bindings[binding];

  vk::WriteDescriptorSet write{
      .dstBinding = binding,
      .descriptorCount = 1,
      .descriptorType = binding_desc.descriptorType,
      .pImageInfo = imageInfo,
  };

  m_writes.push_back(write);
  return *this;
}

std::unique_ptr<VulkanDescriptorSet> DescriptorWriter::build() {
  vk::raii::DescriptorSet ds =
      m_ds_pool.allocateDescriptor(m_ds_layout.getDescriptorSetLayout());
  overwrite(ds);
  return std::make_unique<VulkanDescriptorSet>(m_ds_pool.m_device,
                                               std::move(ds));
}

void DescriptorWriter::overwrite(vk::raii::DescriptorSet &set) {
  for (auto &write : m_writes) {
    write.dstSet = set;
  }
  m_ds_pool.m_device.getHandle().updateDescriptorSets(m_writes, {});
}

VulkanDescriptorSet::VulkanDescriptorSet(VulkanDevice &device,
                                         vk::raii::DescriptorSet ds)
    : m_device(device), m_decriptor_set(std::move(ds)) {}

VulkanDescriptorSet::~VulkanDescriptorSet() {}

} // namespace ssme::vulkan
