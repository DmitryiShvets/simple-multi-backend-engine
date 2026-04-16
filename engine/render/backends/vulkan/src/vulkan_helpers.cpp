#include "vulkan_helpers.h"

namespace ssme::vulkan {

bool isDepthFormat(vk::Format format) {
  return format == vk::Format::eD32Sfloat ||
         format == vk::Format::eD32SfloatS8Uint ||
         format == vk::Format::eD24UnormS8Uint ||
         format == vk::Format::eD16Unorm ||
         format == vk::Format::eD16UnormS8Uint;
}
vk::ImageLayout toVkImageLayout(ImageLayout layout) {
  switch (layout) {
  case ImageLayout::UNDEFINED:
    return vk::ImageLayout::eUndefined;
  case ImageLayout::COLOR_ATTACHMENT:
    return vk::ImageLayout::eColorAttachmentOptimal;
  case ImageLayout::PRESENT_SRC:
    return vk::ImageLayout::ePresentSrcKHR;
  case ImageLayout::TRANSFER_DST:
    return vk::ImageLayout::eTransferDstOptimal;
  case ImageLayout::SHADER_READ_ONLY:
    return vk::ImageLayout::eShaderReadOnlyOptimal;
  default:
    return vk::ImageLayout::eUndefined;
  }
}
vk::Format toVkFormat(Format format) {
  switch (format) {
  case Format::R32G32B32_SFLOAT:
    return vk::Format::eR32G32B32Sfloat;
  case Format::R32G32_SFLOAT:
    return vk::Format::eR32G32Sfloat;
  default:
    return vk::Format::eUndefined;
  }
}
vk::DescriptorType toVkDescriptorType(DescriptorType type) {
  switch (type) {
  case DescriptorType::UNIFORM_BUFFER:
    return vk::DescriptorType::eUniformBuffer;
  case DescriptorType::COMBINED_IMAGE_SAMPLER:
    return vk::DescriptorType::eCombinedImageSampler;
    // Add other types as needed
  default:
    throw std::runtime_error("Unsupported descriptor type");
  }
}
vk::ShaderStageFlags toVkShaderStageFlags(ShaderStageFlags flags) {
  vk::ShaderStageFlags shader_flags;
  if (flags & static_cast<uint32_t>(ShaderStage::VERTEX)) {
    shader_flags |= vk::ShaderStageFlagBits::eVertex;
  }
  if (flags & static_cast<uint32_t>(ShaderStage::FRAGMENT)) {
    shader_flags |= vk::ShaderStageFlagBits::eFragment;
  }
  if (flags & static_cast<uint32_t>(ShaderStage::COMPUTE)) {
    shader_flags |= vk::ShaderStageFlagBits::eCompute;
  }
  if (!shader_flags) {
    throw std::runtime_error("Unsupported shader type");
  }
  return shader_flags;
}
vk::BufferUsageFlags toVkBufferUsageFlags(BufferUsageFlags usage) {
  vk::BufferUsageFlags usage_flags;

  if (usage & static_cast<uint32_t>(BufferUsage::VERTEX_BUFFER)) {
    usage_flags |= vk::BufferUsageFlagBits::eVertexBuffer;
  }
  if (usage & static_cast<uint32_t>(BufferUsage::UNIFORM_BUFFER)) {
    usage_flags |= vk::BufferUsageFlagBits::eUniformBuffer;
  }
  if (usage & static_cast<uint32_t>(BufferUsage::INDEX_BUFFER)) {
    usage_flags |= vk::BufferUsageFlagBits::eIndexBuffer;
  }
  if (!usage_flags) {
    throw std::runtime_error("Unsupported buffer type");
  }
  return usage_flags;
}

vk::IndexType toVkIndexType(IndexType type) {
  switch (type) {
  case IndexType::UINT16:
    return vk::IndexType::eUint16;
  case IndexType::UINT32:
    return vk::IndexType::eUint32;
  default:
    return vk::IndexType::eUint32;
  }
}
} // namespace ssme::vulkan
