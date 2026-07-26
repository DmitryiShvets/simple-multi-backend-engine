#include "vulkan_texture.h"
#include <vulkan_device.h>
#include "utils/image_loader.h"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace ssme::vulkan {

// File-loading constructor
VulkanTexture::VulkanTexture(VulkanDevice &device, const std::string &filepath)
    : m_device(device), m_is_owned(true) {
  createTextureImage(filepath);
  createTextureImageView(vk::Format::eR8G8B8A8Srgb,
                         vk::ImageAspectFlagBits::eColor);
  createTextureSampler();
}

// Swapchain-wrapping constructor
VulkanTexture::VulkanTexture(VulkanDevice &device, vk::Image image,
                             vk::Format format)
    : m_device(device), m_borrowed_image(image), m_is_owned(false) {
  createTextureImageView(format, vk::ImageAspectFlagBits::eColor);
  // Sampler is not created for swapchain images, as they are render targets.
}
// Depth image constructor
VulkanTexture::VulkanTexture(VulkanDevice &device, vk::Extent2D extent,
                             vk::Format format)
    : m_device(device), m_is_owned(true) {
  createDepthTextureImage(extent, format);
  createTextureImageView(format, vk::ImageAspectFlagBits::eDepth);
}

VulkanTexture::~VulkanTexture() {}

void VulkanTexture::createTextureSampler() {
  m_samplaer = std::move(m_device.createTextureSampler());
}

void VulkanTexture::createTextureImageView(vk::Format format,
                                           vk::ImageAspectFlagBits flags) {
  m_format = format;
  vk::Image image = m_is_owned ? *m_owned_image.value() : m_borrowed_image;
  m_image_view = m_device.createImageView(image, m_format, flags);
}

void VulkanTexture::createTextureImage(const std::string &filepath) {
  int tex_width, tex_height, tex_channels;
  auto pixels =
      loadImage(filepath.c_str(), &tex_width, &tex_height, &tex_channels);
  vk::DeviceSize image_size = tex_width * tex_height * 4;

  if (!pixels) {
    throw std::runtime_error("Failed to load texture image!");
  }

  auto [staging_buffer, staging_buffer_memory] =
      m_device.createBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc,
                            vk::MemoryPropertyFlagBits::eHostVisible |
                                vk::MemoryPropertyFlagBits::eHostCoherent);

  void *data = staging_buffer_memory.mapMemory(0, image_size);
  memcpy(data, pixels, static_cast<size_t>(image_size));
  staging_buffer_memory.unmapMemory();

  freeImage(pixels);

  vk::ImageCreateInfo create_info{
      .imageType = vk::ImageType::e2D,
      .format = vk::Format::eR8G8B8A8Srgb,
      .extent = vk::Extent3D{.width = static_cast<uint32_t>(tex_width),
                             .height = static_cast<uint32_t>(tex_height),
                             .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eTransferDst |
               vk::ImageUsageFlagBits::eSampled,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined,
  };

  auto [image, image_memory] = m_device.createImage(
      create_info, vk::MemoryPropertyFlagBits::eDeviceLocal);

  m_owned_image = std::move(image);
  m_owned_image_memory = std::move(image_memory);

  m_device.transitionImageLayout(*m_owned_image.value(), vk::Format::eR8G8B8A8Srgb,
                                 vk::ImageLayout::eUndefined,
                                 vk::ImageLayout::eTransferDstOptimal);
  m_device.copyBufferToImage(*staging_buffer, *m_owned_image.value(),
                             static_cast<uint32_t>(tex_width),
                             static_cast<uint32_t>(tex_height), 1);
  m_device.transitionImageLayout(*m_owned_image.value(), vk::Format::eR8G8B8A8Srgb,
                                 vk::ImageLayout::eTransferDstOptimal,
                                 vk::ImageLayout::eShaderReadOnlyOptimal);
}

void VulkanTexture::createDepthTextureImage(vk::Extent2D extent,
                                            vk::Format format) {
  vk::ImageCreateInfo create_info{
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = vk::Extent3D{.width = static_cast<uint32_t>(extent.width),
                             .height = static_cast<uint32_t>(extent.height),
                             .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined,
  };

  auto [image, image_memory] = m_device.createImage(
      create_info, vk::MemoryPropertyFlagBits::eDeviceLocal);

  m_owned_image = std::move(image);
  m_owned_image_memory = std::move(image_memory);
  // Transition depth image from UNDEFINED to DEPTH_ATTACHMENT_OPTIMAL
  m_device.transitionImageLayout(*m_owned_image.value(), format,
                                 vk::ImageLayout::eUndefined,
                                 vk::ImageLayout::eDepthAttachmentOptimal);
}

} // namespace ssme::vulkan
