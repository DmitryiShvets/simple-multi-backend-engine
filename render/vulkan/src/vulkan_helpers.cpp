#include "vulkan_helpers.h"

namespace Render::Vulkan {

bool isDepthFormat(vk::Format format) {
  return format == vk::Format::eD32Sfloat ||
         format == vk::Format::eD32SfloatS8Uint ||
         format == vk::Format::eD24UnormS8Uint ||
         format == vk::Format::eD16Unorm ||
         format == vk::Format::eD16UnormS8Uint;
}
vk::ImageLayout toVkImageLayout(Render::ImageLayout layout) {
  switch (layout) {
  case Render::ImageLayout::UNDEFINED:
    return vk::ImageLayout::eUndefined;
  case Render::ImageLayout::COLOR_ATTACHMENT:
    return vk::ImageLayout::eColorAttachmentOptimal;
  case Render::ImageLayout::PRESENT_SRC:
    return vk::ImageLayout::ePresentSrcKHR;
  case Render::ImageLayout::TRANSFER_DST:
    return vk::ImageLayout::eTransferDstOptimal;
  case Render::ImageLayout::SHADER_READ_ONLY:
    return vk::ImageLayout::eShaderReadOnlyOptimal;
  default:
    return vk::ImageLayout::eUndefined;
  }
}
vk::Format toVkFormat(Core::Format format) {
  switch (format) {
  case Core::Format::R32G32B32_SFLOAT:
    return vk::Format::eR32G32B32Sfloat;
  case Core::Format::R32G32_SFLOAT:
    return vk::Format::eR32G32Sfloat;
  default:
    return vk::Format::eUndefined;
  }
}
vk::DescriptorType toVkDescriptorType(Render::DescriptorType type) {
  switch (type) {
  case Render::DescriptorType::UNIFORM_BUFFER:
    return vk::DescriptorType::eUniformBuffer;
  case Render::DescriptorType::COMBINED_IMAGE_SAMPLER:
    return vk::DescriptorType::eCombinedImageSampler;
    // Add other types as needed
  default:
    throw std::runtime_error("Unsupported descriptor type");
  }
}
vk::ShaderStageFlags toVkShaderStageFlags(Render::ShaderStageFlags flags) {
  vk::ShaderStageFlags shader_flags;
  if (flags & static_cast<uint32_t>(Render::ShaderStage::VERTEX)) {
    shader_flags |= vk::ShaderStageFlagBits::eVertex;
  }
  if (flags & static_cast<uint32_t>(Render::ShaderStage::FRAGMENT)) {
    shader_flags |= vk::ShaderStageFlagBits::eFragment;
  }
  if (flags & static_cast<uint32_t>(Render::ShaderStage::COMPUTE)) {
    shader_flags |= vk::ShaderStageFlagBits::eCompute;
  }
  if (!shader_flags) {
    throw std::runtime_error("Unsupported shader type");
  }
  return shader_flags;
}
vk::BufferUsageFlags toVkBufferUsageFlags(Render::BufferUsageFlags usage) {
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
} // namespace Render::Vulkan
