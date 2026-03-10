#pragma once
#include "vulkan_device.h"

namespace Render::Vulkan {
class VulkanPipelineLayout {
public:
    VulkanPipelineLayout(VulkanDevice& device, vk::raii::PipelineLayout layout);
    ~VulkanPipelineLayout();

    // Non-copyable
    VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;
    VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;

    vk::PipelineLayout getHandle() const { return *m_layout; }
private:
    VulkanDevice& m_device;
    vk::raii::PipelineLayout m_layout = nullptr;
};
}
