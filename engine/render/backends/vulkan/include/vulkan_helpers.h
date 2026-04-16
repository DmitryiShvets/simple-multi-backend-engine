#pragma once

#include "core/render_types.h"
#include "vulkan/vulkan.hpp"

namespace ssme::vulkan {

bool isDepthFormat(vk::Format format);
vk::ImageLayout toVkImageLayout(ImageLayout layout);
vk::Format toVkFormat(Format format);
vk::DescriptorType toVkDescriptorType(DescriptorType type);
vk::ShaderStageFlags toVkShaderStageFlags(ShaderStageFlags flags);
vk::BufferUsageFlags toVkBufferUsageFlags(BufferUsageFlags usage);
vk::IndexType toVkIndexType(IndexType type);
} // namespace ssme::vulkan
