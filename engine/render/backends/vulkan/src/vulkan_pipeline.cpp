#include "vulkan_pipeline.h"
#include "vulkan_shader_module.h"
#include <cassert>
#include <memory>
#include <utility>

namespace ssme::vulkan {
// =================================================================================================
// PipelineConfig::Builder Implementation
// =================================================================================================

PipelineConfigInfo::Builder::Builder() {
  // This constructor now sets all the default values, replacing the old static
  // function.
  m_config = std::make_unique<PipelineConfigInfo>();

  // Set all default values on the heap-allocated object
  m_config->input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo{
      .topology = vk::PrimitiveTopology::eTriangleList,
      .primitiveRestartEnable = vk::False,
  };
  m_config->viewport_info = vk::PipelineViewportStateCreateInfo{
      .viewportCount = 1,
      .scissorCount = 1,
  };
  m_config->rasterization_info = vk::PipelineRasterizationStateCreateInfo{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      // .polygonMode = vk::PolygonMode::eLine, //wireframe mode
      .cullMode = vk::CullModeFlagBits::eNone,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasEnable = vk::False,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp = 0.0f,
      .depthBiasSlopeFactor = 0.0f,
      .lineWidth = 1.0f,

  };
  m_config->multisample_info = vk::PipelineMultisampleStateCreateInfo{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = vk::False,
      .minSampleShading = 1.0f,
      .pSampleMask = {},
      .alphaToCoverageEnable = vk::False,
      .alphaToOneEnable = vk::False};
  m_config->depth_stencil_info = vk::PipelineDepthStencilStateCreateInfo{
      .depthTestEnable = vk::True,
      .depthWriteEnable = vk::True,
      .depthCompareOp = vk::CompareOp::eLess,
      .depthBoundsTestEnable = vk::False,
      .stencilTestEnable = vk::False,
      .minDepthBounds = 0.0f,
      .maxDepthBounds = 1.0f,
  };
  m_config->color_blend_attachment = vk::PipelineColorBlendAttachmentState{
      .blendEnable = vk::False,
      .srcColorBlendFactor = vk::BlendFactor::eOne,
      .colorBlendOp = vk::BlendOp::eAdd,
      .srcAlphaBlendFactor = vk::BlendFactor::eOne,
      .dstAlphaBlendFactor = vk::BlendFactor::eZero,
      .alphaBlendOp = vk::BlendOp::eAdd,
      .colorWriteMask =
          (vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA),
  };
  m_config->color_blend_info = vk::PipelineColorBlendStateCreateInfo{
      .logicOpEnable = vk::False,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &m_config->color_blend_attachment};

  m_config->dynamic_states = {vk::DynamicState::eViewport,
                              vk::DynamicState::eScissor};
  // this state can actually be changed without recreating the pipeline at draw
  // time
  m_config->dynamic_state_info = vk::PipelineDynamicStateCreateInfo{
      .dynamicStateCount = static_cast<uint32_t>(m_config->dynamic_states.size()),
      .pDynamicStates = m_config->dynamic_states.data()};
}
// Default special members for PIMPL
PipelineConfigInfo::Builder::~Builder() = default;
PipelineConfigInfo::Builder::Builder(Builder &&) noexcept = default;
PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::operator=(Builder &&) noexcept = default;

std::unique_ptr<PipelineConfigInfo> PipelineConfigInfo::Builder::build() {
  return std::move(m_config);
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::setPipelineLayout(vk::PipelineLayout layout) {
  m_config->pipeline_layout = layout;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::setColorAttachmentFormats(
    const std::vector<vk::Format> &formats) {
  m_config->dynamic_color_attachment_formats = formats;
  return *this;
}

PipelineConfigInfo::Builder &PipelineConfigInfo::Builder::setVertexInputInfo(
    const std::vector<vk::VertexInputBindingDescription> &binding_decription,
    const std::vector<vk::VertexInputAttributeDescription> &attrib_decription) {
  m_config->vertex_input_info = vk::PipelineVertexInputStateCreateInfo{
      .vertexBindingDescriptionCount =
          static_cast<uint32_t>(binding_decription.size()),
      .pVertexBindingDescriptions = binding_decription.data(),
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attrib_decription.size()),
      .pVertexAttributeDescriptions = attrib_decription.data()};
  return *this;
}

PipelineConfigInfo::Builder &PipelineConfigInfo::Builder::setPrimitiveTopology(
    vk::PrimitiveTopology topology) {
  m_config->input_assembly_info.topology = topology;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::setPolygonMode(vk::PolygonMode mode) {
  m_config->rasterization_info.polygonMode = mode;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::setCullMode(vk::CullModeFlags cullMode) {
  m_config->rasterization_info.cullMode = cullMode;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::setFrontFace(vk::FrontFace frontFace) {
  m_config->rasterization_info.frontFace = frontFace;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::enableDepthTest(bool enable) {
  m_config->depth_stencil_info.depthTestEnable = enable ? vk::True : vk::False;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::enableDepthWrite(bool enable) {
  m_config->depth_stencil_info.depthWriteEnable = enable ? vk::True : vk::False;
  return *this;
}

// =================================================================================================
// VulkanPipeLine Implementation
// =================================================================================================

VulkanPipeLine::VulkanPipeLine(VulkanDevice &device,
                               const PipelineConfigInfo &config,
                               const VulkanShaderModule &vert_shader,
                               const VulkanShaderModule &frag_shader)
    : m_device(device), m_pipeline_layout(config.pipeline_layout) {
  createGraphicsPipeline(vert_shader, frag_shader, config);
}

VulkanPipeLine::~VulkanPipeLine() {}

void VulkanPipeLine::createGraphicsPipeline(
    const VulkanShaderModule &vert_shader,
    const VulkanShaderModule &frag_shader, const PipelineConfigInfo &config) {
  assert(
      config.pipeline_layout != nullptr &&
      "cannot create graphics pipeline : no pipelineLayout provided in config");

  // VulkanShaderModule vert_shader_module = VulkanShaderModule(m_device, vert_shader_filepath);
  // VulkanShaderModule frag_shader_module = VulkanShaderModule(m_device, frag_shader_filepath);

  vk::PipelineShaderStageCreateInfo vert_shader_stages{

      .stage = vk::ShaderStageFlagBits::eVertex,
      .module = vert_shader.getHandle(),
      .pName = "main"};
  vk::PipelineShaderStageCreateInfo frag_shader_stages{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = frag_shader.getHandle(),
      .pName = "main"};

  vk::PipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stages,
                                                       frag_shader_stages};

  // --- Dynamic Rendering Setup ---
  vk::PipelineRenderingCreateInfo rendering_create_info{
      .colorAttachmentCount =
          static_cast<uint32_t>(config.dynamic_color_attachment_formats.size()),
      .pColorAttachmentFormats = config.dynamic_color_attachment_formats.data(),
      // TODO: Set depth/stencil formats here later if needed
      .depthAttachmentFormat = vk::Format::eD32Sfloat,
      .stencilAttachmentFormat = ::vk::Format::eUndefined};

  vk::GraphicsPipelineCreateInfo create_info{
      .pNext = &rendering_create_info,
      .stageCount = 2,
      .pStages = shader_stages,
      .pVertexInputState = &config.vertex_input_info,
      .pInputAssemblyState = &config.input_assembly_info,
      .pViewportState = &config.viewport_info,
      .pRasterizationState = &config.rasterization_info,
      .pMultisampleState = &config.multisample_info,
      .pDepthStencilState = &config.depth_stencil_info,
      .pColorBlendState = &config.color_blend_info,
      .pDynamicState = &config.dynamic_state_info,
      .layout = config.pipeline_layout,
      .renderPass = nullptr, // Must be null for dynamic rendering
      .subpass = 0,
      .basePipelineHandle = nullptr,
      .basePipelineIndex = -1,
  };
  m_graphics_pipeline =
      vk::raii::Pipeline(m_device.getHandle(), nullptr, create_info);
}

} // namespace ssme::vulkan
