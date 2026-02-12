#pragma once
#include "vulkan_device.h"
#include <vulkan/vulkan.h>

namespace Render::Vulkan {
class VulkanPipelineLayout {
public:
    VulkanPipelineLayout(VulkanDevice& device, VkPipelineLayout layout);
    ~VulkanPipelineLayout();

    // Non-copyable
    VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;
    VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;

    VkPipelineLayout get() const { return m_layout; }
private:
    VulkanDevice& m_device;
    VkPipelineLayout m_layout;
};
}
