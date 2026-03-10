#include "vulkan_pipeline_layout.h"
#include <utility>

namespace Render::Vulkan {

VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice &device,
                                           vk::raii::PipelineLayout layout)
    : m_device(device), m_layout(std::move(layout)) {}

VulkanPipelineLayout::~VulkanPipelineLayout() {}

} // namespace Render::Vulkan
