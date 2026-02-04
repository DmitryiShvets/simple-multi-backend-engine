#include "vulkan_rhi_device.h"

#include "vulkan_device.h"
#include <utility>

namespace Render::Vulkan {

VulkanRHIDevice::VulkanRHIDevice(
    std::unique_ptr<VulkanDevice> device)
    : m_device(std::move(device)) {
  // The constructor is now simple, only taking ownership of the low-level device.
}

VulkanRHIDevice::~VulkanRHIDevice() {
  // Destruction is handled by unique_ptr.
}

void VulkanRHIDevice::free(RID rid) {
    m_resource_manager.free(rid);
}

void VulkanRHIDevice::tick() {
  // This is where deferred resource cleanup will be handled.
}

void VulkanRHIDevice::waitIdle() {
  // This is a low-level device command, so it stays here.
  vkDeviceWaitIdle(m_device->getDeviceHandle());
}

} // namespace Render::Vulkan
