#pragma once
#include <vulkan_device.h>
#include <string>
#include <vulkan/vulkan.h>

namespace Render::Vulkan {

class VulkanTexture {
public:
  // Constructor for loading from file (owns the image)
  VulkanTexture(VulkanDevice &device, const std::string &filepath);
  // Constructor for wrapping a swapchain image (does not own the image)
  VulkanTexture(VulkanDevice &device, VkImage image, VkFormat format);
  // Constructor for creating depth image
  VulkanTexture(VulkanDevice &device, VkExtent2D extent, VkFormat format);
  ~VulkanTexture();

  VulkanTexture(const VulkanTexture &) = delete;
  VulkanTexture &operator=(const VulkanTexture &) = delete;

  VkDescriptorImageInfo imageInfo{};

  // Getters
  VkImage getImage() const { return textureImage; }
  VkImageView getImageView() const { return textureImageView; }
  VkSampler getSampler() const { return textureSampler; }

private:
  VulkanDevice &m_device;

  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
  bool m_is_owned;

  void createTextureSampler();
  void createTextureImageView(VkFormat format, VkImageAspectFlagBits flags);
  void createTextureImage(const std::string &filepath);
  void createDepthTextureImage(VkExtent2D extent, VkFormat format);
};

} // namespace Render::Vulkan
