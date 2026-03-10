#pragma once

#include "render_types.h"
#include "vulkan/vulkan.hpp"

namespace Render::Vulkan {

bool isDepthFormat(vk::Format format);
vk::ImageLayout toVkImageLayout(Render::ImageLayout layout);
vk::Format toVkFormat(Core::Format format);
vk::DescriptorType toVkDescriptorType(Render::DescriptorType type);
vk::ShaderStageFlags toVkShaderStageFlags(Render::ShaderStageFlags flags);
vk::BufferUsageFlags toVkBufferUsageFlags(Render::BufferUsageFlags usage);
} // namespace Render::Vulkan
