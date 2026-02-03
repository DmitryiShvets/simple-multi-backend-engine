#include "vulkan_rendering_device.h"

// We need to include concrete implementation headers here
// to create the actual resources.
#include "render_types.h"
#include "vulkan_command_list.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_swap_chain.h"
#include <memory>
#include <utility>

namespace Render::Vulkan {

VulkanRenderingDevice::VulkanRenderingDevice(
    std::unique_ptr<VulkanDevice> device)
    : m_device(std::move(device)) {
  m_set_layout = DescriptorSetLayout::Builder(*m_device)
                     .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT)
                     .addBinding(1, VK_DESCRIPTOR_TYPE_SAMPLER,
                                 VK_SHADER_STAGE_FRAGMENT_BIT)
                     .addBinding(2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                 VK_SHADER_STAGE_FRAGMENT_BIT, 2)
                     .build();
  m_test_rs = std::make_unique<SimpleRenderSystem>(
      *m_device, m_swap_chain->getRenderPass(),
      m_set_layout->getDescriptorSetLayout());
}

VulkanRenderingDevice::~VulkanRenderingDevice() {
  // The unique_ptrs in m_command_lists will be automatically destroyed.
  // The unique_ptr for m_device will be automatically destroyed.
}

RID VulkanRenderingDevice::createSwapChain(const SwapChainDesc &desc) {
  // 1. Create the concrete Vulkan resource object.
  // The constructor needs the low-level device to get Vulkan handles.
  // TODO: obtain VkExtent2D from ...
  m_swap_chain = std::make_unique<VulkanSwapChain>(
      *m_device, VkExtent2D{800, 400}, m_resource_manager);

  // TEMPARARY because we now have one command list per one swapchain image we
  // have to handel it
  m_command_lists.clear();
  m_command_lists.reserve(m_swap_chain->getImageCount());
  for (int i = 0; i < 4; i++) {
    m_command_lists.push_back(
        std::make_unique<VulkanCommandList>(*m_device, m_resource_manager));
  }
  // 2. Add it to the resource manager, which returns a RID.
  // The manager now owns the object.
  // return m_resource_manager.add(std::move(swap_chain));
}

void VulkanRenderingDevice::free(RID rid) { m_resource_manager.free(rid); }

void VulkanRenderingDevice::tick() {
  // This is where we can perform deferred resource cleanup in the future.
}

CommandList *VulkanRenderingDevice::beginCommandList(RID swapChain) {
  auto vk_swapchain = m_resource_manager.get_ptr<VulkanSwapChain>(swapChain);
  return m_command_lists[vk_swapchain->getCurrentFrameIndex()].get();
}

void VulkanRenderingDevice::submitCommandList(RID swapChain,
                                              CommandList *list) {
  // Cast the interface pointer back to the concrete Vulkan implementation.
  auto vk_command_list = static_cast<VulkanCommandList *>(list);
  VkCommandBuffer buffer = vk_command_list->getHandle();
  auto vk_swapchain = m_resource_manager.get_ptr<VulkanSwapChain>(swapChain);

  // Use the correct, acquired image index for presentation.
  vk_swapchain->submitCommandBuffers(&buffer, &m_acquired_image_index);
}

RID VulkanRenderingDevice::acquireNextFrame(RID swapChain) {
  // TODO ADD CHECKS
  auto vk_swapchain = m_resource_manager.get_ptr<VulkanSwapChain>(swapChain);
  // The index of the swapchain image we can render to is acquired here.
  vk_swapchain->acquireNextImage(&m_acquired_image_index);
  // Now we get the RID that corresponds to that image index.
  return vk_swapchain->getImageRid(m_acquired_image_index);
}
void VulkanRenderingDevice::present(RID swapChain) {
  // TODO: Implement actual presentation logic using VulkanSwapChain.
}

void VulkanRenderingDevice::waitIdle() {
  vkDeviceWaitIdle(m_device->getDeviceHandle());
}

} // namespace Render::Vulkan
