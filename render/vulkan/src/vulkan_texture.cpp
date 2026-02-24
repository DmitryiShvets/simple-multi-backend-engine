#include "vulkan_texture.h"
#include "image_loader.h"
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace Render::Vulkan {

// File-loading constructor
VulkanTexture::VulkanTexture(VulkanDevice &device, const std::string &filepath)
    : m_device(device), m_is_owned(true) {
  createTextureImage(filepath);
  createTextureImageView(
      VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT); // Assuming this format for loaded files
  createTextureSampler();

  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = textureImageView;
  imageInfo.sampler = textureSampler;
}

// Swapchain-wrapping constructor
VulkanTexture::VulkanTexture(VulkanDevice &device, VkImage image,
                             VkFormat format)
    : m_device(device), textureImage(image), textureImageMemory(VK_NULL_HANDLE),
      textureSampler(VK_NULL_HANDLE), m_is_owned(false) {
  createTextureImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
  // Sampler is not created for swapchain images, as they are render targets.
}
// Depth image constructor
VulkanTexture::VulkanTexture(VulkanDevice &device, VkExtent2D extent,
                             VkFormat format)
    : m_device(device), textureImage(VK_NULL_HANDLE),
      textureImageMemory(VK_NULL_HANDLE), textureSampler(VK_NULL_HANDLE),
      m_is_owned(true) {
  createDepthTextureImage(extent, format);
  createTextureImageView(format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

VulkanTexture::~VulkanTexture() {
  if (textureSampler != VK_NULL_HANDLE) {
    vkDestroySampler(m_device.getDeviceHandle(), textureSampler, nullptr);
  }
  if (textureImageView != VK_NULL_HANDLE) {
    vkDestroyImageView(m_device.getDeviceHandle(), textureImageView, nullptr);
  }

  // Only destroy the image and memory if we own it
  if (m_is_owned) {
    if (textureImage != VK_NULL_HANDLE) {
      vkDestroyImage(m_device.getDeviceHandle(), textureImage, nullptr);
    }
    if (textureImageMemory != VK_NULL_HANDLE) {
      vkFreeMemory(m_device.getDeviceHandle(), textureImageMemory, nullptr);
    }
  }
}

void VulkanTexture::createTextureSampler() {
  m_device.createTextureSampler(textureSampler);
}

void VulkanTexture::createTextureImageView(VkFormat format, VkImageAspectFlagBits flags) {
  textureImageView = m_device.createImageView(textureImage, format, flags);
}

void VulkanTexture::createTextureImage(const std::string &filepath) {
  int texWidth, texHeight, texChannels;
  auto pixels =
      loadImage(filepath.c_str(), &texWidth, &texHeight, &texChannels);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  m_device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(m_device.getDeviceHandle(), stagingBufferMemory, 0, imageSize, 0,
              &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(m_device.getDeviceHandle(), stagingBufferMemory);

  freeImage(pixels);

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32_t>(texWidth);
  imageInfo.extent.height = static_cast<uint32_t>(texHeight);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage =
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  m_device.createImage(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       textureImage, textureImageMemory);

  m_device.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                                 VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  m_device.copyBufferToImage(stagingBuffer, textureImage,
                             static_cast<uint32_t>(texWidth),
                             static_cast<uint32_t>(texHeight), 1);
  m_device.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(m_device.getDeviceHandle(), stagingBuffer, nullptr);
  vkFreeMemory(m_device.getDeviceHandle(), stagingBufferMemory, nullptr);
}

void VulkanTexture::createDepthTextureImage(VkExtent2D extent,
                                            VkFormat format) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = extent.width;
  imageInfo.extent.height = extent.height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.flags = 0;
  m_device.createImage(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       textureImage, textureImageMemory);
  
  // Transition depth image from UNDEFINED to DEPTH_ATTACHMENT_OPTIMAL
  m_device.transitionImageLayout(textureImage, format,
                                 VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
}

} // namespace Render::Vulkan
