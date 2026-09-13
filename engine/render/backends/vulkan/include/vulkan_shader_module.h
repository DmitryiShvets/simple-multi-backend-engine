#pragma once
#include "vulkan_device.h"
#include <string>
#include <vector>

namespace ssme::vulkan {
class VulkanShaderModule {
public:
  VulkanShaderModule(VulkanDevice &device, const std::string & shader_filepath);
  VulkanShaderModule(VulkanDevice &device, const std::vector<char> & code);
  ~VulkanShaderModule();

  // Non-copyable
  VulkanShaderModule(const VulkanShaderModule &) = delete;
  VulkanShaderModule &operator=(const VulkanShaderModule &) = delete;

  vk::ShaderModule getHandle() const { return *m_shader_module; }

private:
  [[nodiscard]]
  vk::raii::ShaderModule createShaderModule(const std::vector<char> &code);

  VulkanDevice &m_device;
  vk::raii::ShaderModule m_shader_module = nullptr;
};
} // namespace ssme::vulkan
