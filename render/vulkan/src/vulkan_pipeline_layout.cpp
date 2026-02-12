#include "vulkan_pipeline_layout.h"

namespace Render::Vulkan {

VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice& device, VkPipelineLayout layout)
    : m_device(device), m_layout(layout) {}

VulkanPipelineLayout::~VulkanPipelineLayout() {
    if (m_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device.getDeviceHandle(), m_layout, nullptr);
    }
}

}
