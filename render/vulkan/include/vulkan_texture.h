#pragma once
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan_device.h>

namespace Render::Vulkan {

class VulkanTexture {
public:
  // Constructor for loading from file (owns the image)
  VulkanTexture(VulkanDevice &device, const std::string &filepath);
  // Constructor for wrapping a swapchain image (does not own the image)
  VulkanTexture(VulkanDevice &device, vk::Image image, vk::Format format);
  // Constructor for creating depth image
  VulkanTexture(VulkanDevice &device, vk::Extent2D extent,vk::Format format);
  ~VulkanTexture();

  VulkanTexture(const VulkanTexture &) = delete;
  VulkanTexture &operator=(const VulkanTexture &) = delete;

  // Getters
  vk::Format getFormat() const { return m_format; }
  vk::ImageView getImageView() const { return m_image_view; }
  vk::Sampler getSampler() const { return *m_samplaer; }
  vk::Image getImage() const {
    return m_is_owned ? *m_owned_image : m_borrowed_image;
  }
private:
  VulkanDevice &m_device;

  std::optional<vk::raii::Image> m_owned_image = nullptr;
  vk::Image m_borrowed_image = nullptr; // from swapchain
  std::optional<vk::raii::DeviceMemory> m_owned_image_memory = nullptr;
  vk::raii::ImageView m_image_view = nullptr;
  vk::raii::Sampler m_samplaer = nullptr;
  vk::Format m_format;
  bool m_is_owned;

  void createTextureSampler();
  void createTextureImageView(vk::Format format, vk::ImageAspectFlagBits flags);
  void createTextureImage(const std::string &filepath);
  void createDepthTextureImage(vk::Extent2D extent, vk::Format format);
};

} // namespace Render::Vulkan
