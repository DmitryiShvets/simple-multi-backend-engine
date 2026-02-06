#pragma once
#include <vulkan_device.h>
#include <string>

namespace Render::Vulkan {

class VulkanTexture {
public:
  VulkanTexture(VulkanDevice &device, const std::string &filepath);
  ~VulkanTexture();

  VkDescriptorImageInfo imageInfo{};

private:
  VulkanDevice &m_device;

  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;

  void createTextureSampler();

  void createTextureImageView();

  void createTextureImage(const std::string &filepath);
};

} // namespace Render::Vulkan
