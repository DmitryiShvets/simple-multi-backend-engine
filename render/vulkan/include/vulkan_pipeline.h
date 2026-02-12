#pragma once
#include "vulkan_device.h"
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Render::Vulkan {

struct PipelineConfigInfo {
public:
  class Builder {
  public:
    Builder();
    ~Builder(); // Destructor for PIMPL

    Builder(Builder &&) noexcept;
    Builder &operator=(Builder &&) noexcept;

    // Fluent interface setters
    Builder &setPipelineLayout(VkPipelineLayout layout);
    Builder &setColorAttachmentFormats(const std::vector<VkFormat>& formats);
    Builder &setVertexInputInfo(
        const std::vector<VkVertexInputBindingDescription> &binding_desc,
        const std::vector<VkVertexInputAttributeDescription> &attrib_desc);
    Builder &setPrimitiveTopology(VkPrimitiveTopology topology);
    Builder &setPolygonMode(VkPolygonMode mode);
    Builder &setCullMode(VkCullModeFlags cullMode);
    Builder &setFrontFace(VkFrontFace frontFace);
    Builder &enableDepthTest(bool enable);
    Builder &enableDepthWrite(bool enable);

    std::unique_ptr<PipelineConfigInfo> build();

  private:
    std::unique_ptr<PipelineConfigInfo> m_config;
  };

  PipelineConfigInfo() = default;
  PipelineConfigInfo(const PipelineConfigInfo &) = delete;
  PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
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
  std::vector<VkFormat> colorAttachmentFormats;
};

class VulkanPipeLine {
public:
  // Main constructor now takes the config object
  VulkanPipeLine(VulkanDevice &device, const PipelineConfigInfo &config,
                 const std::string &vert_shader_filepath,
                 const std::string &frag_shader_filepath);

  ~VulkanPipeLine();

  void bind_buffer(VkCommandBuffer buffer);
  VkPipeline getHandle() const { return m_graphics_pipeline; }
  VulkanPipeLine(const VulkanPipeLine &) = delete;
  VulkanPipeLine &operator=(const VulkanPipeLine &) = delete;

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
