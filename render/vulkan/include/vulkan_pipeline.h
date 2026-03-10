#pragma once
#include "vulkan_device.h"

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

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
    Builder &setPipelineLayout(vk::PipelineLayout layout);
    Builder &setColorAttachmentFormats(const std::vector<vk::Format> &formats);
    Builder &setVertexInputInfo(
        const std::vector<vk::VertexInputBindingDescription> &binding_desc,
        const std::vector<vk::VertexInputAttributeDescription> &attrib_desc);
    Builder &setPrimitiveTopology(vk::PrimitiveTopology topology);
    Builder &setPolygonMode(vk::PolygonMode mode);
    Builder &setCullMode(vk::CullModeFlags cullMode);
    Builder &setFrontFace(vk::FrontFace frontFace);
    Builder &enableDepthTest(bool enable);
    Builder &enableDepthWrite(bool enable);

    std::unique_ptr<PipelineConfigInfo> build();

  private:
    std::unique_ptr<PipelineConfigInfo> m_config;
  };

  PipelineConfigInfo() = default;
  PipelineConfigInfo(const PipelineConfigInfo &) = delete;
  PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

  vk::PipelineDynamicStateCreateInfo dynamic_state_info;
  vk::PipelineVertexInputStateCreateInfo vertex_input_info;
  vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
  vk::PipelineViewportStateCreateInfo viewport_info;
  vk::PipelineRasterizationStateCreateInfo rasterization_info;
  vk::PipelineMultisampleStateCreateInfo multisample_info;
  vk::PipelineDepthStencilStateCreateInfo depth_stencil_info;
  vk::PipelineColorBlendAttachmentState color_blend_attachment;
  vk::PipelineColorBlendStateCreateInfo color_blend_info;
  vk::PipelineLayout pipeline_layout = nullptr;
  std::vector<vk::Format> dynamic_color_attachment_formats;
  std::vector<vk::DynamicState> dynamic_states;
};

class VulkanPipeLine {
public:
  // Main constructor now takes the config object
  VulkanPipeLine(VulkanDevice &device, const PipelineConfigInfo &config,
                 const std::string &vert_shader_filepath,
                 const std::string &frag_shader_filepath);

  ~VulkanPipeLine();

  vk::Pipeline getHandle() const { return *m_graphics_pipeline; }
  vk::PipelineLayout getLayoutHandle() const { return m_pipeline_layout; }
  VulkanPipeLine(const VulkanPipeLine &) = delete;
  VulkanPipeLine &operator=(const VulkanPipeLine &) = delete;

private:
  void createGraphicsPipeline(const std::string &vert_shader_filepath,
                                const std::string &frag_shader_filepath,
                                const PipelineConfigInfo &config);
  [[nodiscard]]
  vk::raii::ShaderModule createShaderModule(const std::vector<char> &code);

  vk::raii::Pipeline m_graphics_pipeline = nullptr;
  vk::PipelineLayout m_pipeline_layout;
  VulkanDevice &m_device;
};

} // namespace Render::Vulkan
