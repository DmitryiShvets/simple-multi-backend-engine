#include "vulkan_shader_module.h"
#include "utils/common_utils.h"
#include <vector>

namespace ssme::vulkan {

VulkanShaderModule::VulkanShaderModule(VulkanDevice &device,
                                       const std::string &shader_filepath)
    : m_device(device) {
  auto code = CUtils::readFileChar(shader_filepath);
  m_shader_module = createShaderModule(code);
}

[[nodiscard]]
vk::raii::ShaderModule
VulkanShaderModule::createShaderModule(const std::vector<char> &code) {
  vk::ShaderModuleCreateInfo create_info{
      .codeSize = code.size() * sizeof(char),
      .pCode = reinterpret_cast<const uint32_t *>(code.data())};

  vk::raii::ShaderModule module{m_device.getHandle(), create_info};
  return module;
}
VulkanShaderModule::~VulkanShaderModule() {}

} // namespace ssme::vulkan
