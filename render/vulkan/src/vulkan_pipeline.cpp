#include "vulkan_pipeline.h"
#include "vulkan_types.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace Render::Vulkan {
// =================================================================================================
// PipelineConfig::Builder Implementation
// =================================================================================================

PipelineConfigInfo::Builder::Builder() {
  // This constructor now sets all the default values, replacing the old static
  // function.
  m_config = std::make_unique<PipelineConfigInfo>();

  // Set all default values on the heap-allocated object
  m_config->inputAssemblyInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  m_config->inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  m_config->inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  m_config->viewportInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  m_config->viewportInfo.viewportCount = 1;
  m_config->viewportInfo.pViewports = nullptr;
  m_config->viewportInfo.scissorCount = 1;
  m_config->viewportInfo.pScissors = nullptr;

  m_config->rasterizationInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  m_config->rasterizationInfo.depthClampEnable = VK_FALSE;
  m_config->rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  m_config->rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  // m_config->rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE; // wireframe mode
  m_config->rasterizationInfo.lineWidth = 1.0f;
  m_config->rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
  m_config->rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  m_config->rasterizationInfo.depthBiasEnable = VK_FALSE;
  m_config->rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
  m_config->rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
  m_config->rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

  m_config->multisampleInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  m_config->multisampleInfo.sampleShadingEnable = VK_FALSE;
  m_config->multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  m_config->multisampleInfo.minSampleShading = 1.0f;          // Optional
  m_config->multisampleInfo.pSampleMask = nullptr;            // Optional
  m_config->multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
  m_config->multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional

  m_config->colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  m_config->colorBlendAttachment.blendEnable = VK_FALSE;
  m_config->colorBlendAttachment.srcColorBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  m_config->colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  m_config->colorBlendAttachment.srcAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  m_config->colorBlendAttachment.dstAlphaBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                       // Optional
  m_config->colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  m_config->colorBlendInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  m_config->colorBlendInfo.logicOpEnable = VK_FALSE;
  m_config->colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
  m_config->colorBlendInfo.attachmentCount = 1;
  m_config->colorBlendInfo.pAttachments = &m_config->colorBlendAttachment;
  m_config->colorBlendInfo.blendConstants[0] = 0.0f; // Optional
  m_config->colorBlendInfo.blendConstants[1] = 0.0f; // Optional
  m_config->colorBlendInfo.blendConstants[2] = 0.0f; // Optional
  m_config->colorBlendInfo.blendConstants[3] = 0.0f; // Optional

  m_config->depthStencilInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  m_config->depthStencilInfo.depthTestEnable = VK_TRUE; // Enable depth test
  m_config->depthStencilInfo.depthWriteEnable = VK_TRUE;
  m_config->depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  m_config->depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  m_config->depthStencilInfo.minDepthBounds = 0.0f; // Optional
  m_config->depthStencilInfo.maxDepthBounds = 1.0f; // Optional
  m_config->depthStencilInfo.stencilTestEnable = VK_FALSE;
  m_config->depthStencilInfo.front = {}; // Optional
  m_config->depthStencilInfo.back = {};  // Optional

  m_config->dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
                                   VK_DYNAMIC_STATE_SCISSOR};
  m_config->dynamicStateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  m_config->dynamicStateInfo.pDynamicStates =
      m_config->dynamicStateEnables.data();
  m_config->dynamicStateInfo.dynamicStateCount =
      static_cast<uint32_t>(m_config->dynamicStateEnables.size());
  m_config->dynamicStateInfo.flags = 0;
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
PipelineConfigInfo::Builder::setPipelineLayout(VkPipelineLayout layout) {
  m_config->pipelineLayout = layout;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::setColorAttachmentFormats(const std::vector<VkFormat>& formats) {
    m_config->colorAttachmentFormats = formats;
    return *this;
}

PipelineConfigInfo::Builder &PipelineConfigInfo::Builder::setVertexInputInfo(
    const std::vector<VkVertexInputBindingDescription> &binding_decription,
    const std::vector<VkVertexInputAttributeDescription> &attrib_decription) {
  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount =
      static_cast<uint32_t>(binding_decription.size());
  vertex_input_info.pVertexBindingDescriptions =
      binding_decription.data(); // Optional
  vertex_input_info.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attrib_decription.size());
  vertex_input_info.pVertexAttributeDescriptions =
      attrib_decription.data(); // Optional
  m_config->vertexInputInfo = vertex_input_info;
  return *this;
}

PipelineConfigInfo::Builder &PipelineConfigInfo::Builder::setPrimitiveTopology(
    VkPrimitiveTopology topology) {
  m_config->inputAssemblyInfo.topology = topology;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::setPolygonMode(VkPolygonMode mode) {
  m_config->rasterizationInfo.polygonMode = mode;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::setCullMode(VkCullModeFlags cullMode) {
  m_config->rasterizationInfo.cullMode = cullMode;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::setFrontFace(VkFrontFace frontFace) {
  m_config->rasterizationInfo.frontFace = frontFace;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::enableDepthTest(bool enable) {
  m_config->depthStencilInfo.depthTestEnable = enable ? VK_TRUE : VK_FALSE;
  return *this;
}

PipelineConfigInfo::Builder &
PipelineConfigInfo::Builder::enableDepthWrite(bool enable) {
  m_config->depthStencilInfo.depthWriteEnable = enable ? VK_TRUE : VK_FALSE;
  return *this;
}

// =================================================================================================
// VulkanPipeLine Implementation
// =================================================================================================

VulkanPipeLine::VulkanPipeLine(VulkanDevice &device,
                               const PipelineConfigInfo &config,
                               const std::string &vert_shader_filepath,
                               const std::string &frag_shader_filepath)
    : m_device(device), m_pipeline_layout(config.pipelineLayout) {
  create_graphics_pipeline(vert_shader_filepath, frag_shader_filepath, config);
}

VulkanPipeLine::~VulkanPipeLine() {
  vkDestroyShaderModule(m_device.getDeviceHandle(), m_vert_shader_module,
                        nullptr);
  vkDestroyShaderModule(m_device.getDeviceHandle(), m_frag_shader_module,
                        nullptr);
  vkDestroyPipeline(m_device.getDeviceHandle(), m_graphics_pipeline, nullptr);
}

void VulkanPipeLine::bind_buffer(VkCommandBuffer buffer) {
  vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_graphics_pipeline);
}

std::vector<char> VulkanPipeLine::read_file(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + filepath);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

void VulkanPipeLine::create_shader_module(const std::vector<char> &code,
                                          VkShaderModule *shader_module) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  if (vkCreateShaderModule(m_device.getDeviceHandle(), &createInfo, nullptr,
                           shader_module) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
}

void VulkanPipeLine::create_graphics_pipeline(
    const std::string &vert_shader_filepath,
    const std::string &frag_shader_filepath, const PipelineConfigInfo &config) {
  assert(
      config.pipelineLayout != VK_NULL_HANDLE &&
      "cannot create graphics pipeline : no pipelineLayout provided in config");

  auto vert_code = read_file(vert_shader_filepath);
  auto frag_code = read_file(frag_shader_filepath);

  create_shader_module(vert_code, &m_vert_shader_module);
  create_shader_module(frag_code, &m_frag_shader_module);

  VkPipelineShaderStageCreateInfo shader_stages[2];
  shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shader_stages[0].module = m_vert_shader_module;
  shader_stages[0].pName = "main";
  shader_stages[0].flags = 0;
  shader_stages[0].pNext = nullptr;
  shader_stages[0].pSpecializationInfo = nullptr;

  shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shader_stages[1].module = m_frag_shader_module;
  shader_stages[1].pName = "main";
  shader_stages[1].flags = 0;
  shader_stages[1].pNext = nullptr;
  shader_stages[1].pSpecializationInfo = nullptr;

  // --- Dynamic Rendering Setup ---
  VkPipelineRenderingCreateInfo rendering_create_info{};
  rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  rendering_create_info.colorAttachmentCount = static_cast<uint32_t>(config.colorAttachmentFormats.size());
  rendering_create_info.pColorAttachmentFormats = config.colorAttachmentFormats.data();
  // TODO: Set depth/stencil formats here later if needed
  rendering_create_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
  rendering_create_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;


  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.pNext = &rendering_create_info; // Chain the dynamic rendering info
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &config.vertexInputInfo;
  pipeline_info.pInputAssemblyState = &config.inputAssemblyInfo;
  pipeline_info.pViewportState = &config.viewportInfo;
  pipeline_info.pRasterizationState = &config.rasterizationInfo;
  pipeline_info.pMultisampleState = &config.multisampleInfo;
  pipeline_info.pDepthStencilState = &config.depthStencilInfo;
  pipeline_info.pColorBlendState = &config.colorBlendInfo;
  pipeline_info.pDynamicState = &config.dynamicStateInfo;

  pipeline_info.layout = config.pipelineLayout;
  pipeline_info.renderPass = VK_NULL_HANDLE; // Must be null for dynamic rendering
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipeline_info.basePipelineIndex = -1;              // Optional

  if (vkCreateGraphicsPipelines(m_device.getDeviceHandle(), VK_NULL_HANDLE, 1,
                                &pipeline_info, nullptr,
                                &m_graphics_pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }
}

} // namespace Render::Vulkan
