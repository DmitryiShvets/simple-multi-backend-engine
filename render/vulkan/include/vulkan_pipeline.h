#pragma once
#include "vulkan_device.h"
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Render::Vulkan {

struct PipelineConfigInfo {
  PipelineConfigInfo() = default;
  PipelineConfigInfo(const PipelineConfigInfo &) = delete;
  PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class VulkanPipeLine {
public:
  VulkanPipeLine(VulkanDevice &device, const PipelineConfigInfo &config,
                 const std::string &vert_shader_filepath,
                 const std::string &frag_shader_filepath);
  VulkanPipeLine(VulkanDevice &device, VkRenderPass render_pass,
                 VkPipelineLayout piplene_layout,
                 const std::string &vert_shader_filepath,
                 const std::string &frag_shader_filepath);
  ~VulkanPipeLine();

  void bind_buffer(VkCommandBuffer buffer);
  VulkanPipeLine(const VulkanPipeLine &) = delete;
  VulkanPipeLine &operator=(const VulkanPipeLine &) = delete;
  static void set_default_config(PipelineConfigInfo &config);

private:
  static std::vector<char> read_file(const std::string &filepath);
  void create_graphics_pipeline(const std::string &vert_shader_filepath,
                                const std::string &frag_shader_filepath,
                                const PipelineConfigInfo &config);
  void create_shader_module(const std::vector<char> &code,
                            VkShaderModule *shader_module);
  VulkanDevice &m_device;
  VkPipeline m_graphics_pipeline;
  VkShaderModule m_vert_shader_module;
  VkShaderModule m_frag_shader_module;
};

} // namespace Render::Vulkan
