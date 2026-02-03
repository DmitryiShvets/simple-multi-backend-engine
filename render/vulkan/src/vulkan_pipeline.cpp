#include "vulkan_pipeline.h"
#include <cassert>
#include <fstream>
#include <iostream>
namespace Render::Vulkan {
VulkanPipeLine::VulkanPipeLine(VulkanDevice &device, VkRenderPass render_pass,
                               VkPipelineLayout piplene_layout,
                               const std::string &vert_shader_filepath,
                               const std::string &frag_shader_filepath)
    : m_device(device) {

  PipelineConfigInfo pipeline_config{};
  set_default_config(pipeline_config);
  pipeline_config.renderPass = render_pass;
  pipeline_config.pipelineLayout = piplene_layout;
  create_graphics_pipeline(vert_shader_filepath, frag_shader_filepath, pipeline_config);
}
VulkanPipeLine::VulkanPipeLine(VulkanDevice &device,
                               const PipelineConfigInfo &config,
                               const std::string &vert_shader_filepath,
                               const std::string &frag_shader_filepath)
    : m_device(device) {
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

void VulkanPipeLine::set_default_config(PipelineConfigInfo &configInfo) {
  configInfo.inputAssemblyInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  configInfo.viewportInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  configInfo.viewportInfo.viewportCount = 1;
  configInfo.viewportInfo.pViewports = nullptr;
  configInfo.viewportInfo.scissorCount = 1;
  configInfo.viewportInfo.pScissors = nullptr;

  configInfo.rasterizationInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
  configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  configInfo.rasterizationInfo.lineWidth = 1.0f;
  configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
  configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
  configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
  configInfo.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
  configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

  configInfo.multisampleInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
  configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  configInfo.multisampleInfo.minSampleShading = 1.0f;          // Optional
  configInfo.multisampleInfo.pSampleMask = nullptr;            // Optional
  configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
  configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional

  configInfo.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
  configInfo.colorBlendAttachment.srcColorBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  configInfo.colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                       // Optional
  configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  configInfo.colorBlendAttachment.srcAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE; // Optional
  configInfo.colorBlendAttachment.dstAlphaBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                       // Optional
  configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  configInfo.colorBlendInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
  configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
  configInfo.colorBlendInfo.attachmentCount = 1;
  configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

  configInfo.depthStencilInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.minDepthBounds = 0.0f; // Optional
  configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
  configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.front = {}; // Optional
  configInfo.depthStencilInfo.back = {};  // Optional

  configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_SCISSOR};
  configInfo.dynamicStateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  configInfo.dynamicStateInfo.pDynamicStates =
      configInfo.dynamicStateEnables.data();
  configInfo.dynamicStateInfo.dynamicStateCount =
      static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
  configInfo.dynamicStateInfo.flags = 0;
}

std::vector<char> VulkanPipeLine::read_file(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
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
      "cannot create graphics pipeline : no pipelineLoyutd provided in config");
  assert(config.renderPass != VK_NULL_HANDLE &&
         "cannot create graphics pipeline : no renderPass provided in config");
  auto vert_code = read_file(vert_shader_filepath);
  auto frag_code = read_file(frag_shader_filepath);

  std::cout << "vert_shader size " << vert_code.size() << std::endl;
  std::cout << "frag_shader size " << frag_code.size() << std::endl;

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

  auto binding_decription = Vertex::get_binding_description();
  auto attrib_decription = Vertex::get_attribute_description();
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

  VkGraphicsPipelineCreateInfo pipeliene_info{};
  pipeliene_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeliene_info.stageCount = 2;
  pipeliene_info.pStages = shader_stages;
  pipeliene_info.pVertexInputState = &vertex_input_info;
  pipeliene_info.pInputAssemblyState = &config.inputAssemblyInfo;
  pipeliene_info.pViewportState = &config.viewportInfo;
  pipeliene_info.pRasterizationState = &config.rasterizationInfo;
  pipeliene_info.pMultisampleState = &config.multisampleInfo;
  pipeliene_info.pDepthStencilState = &config.depthStencilInfo; // Optional
  pipeliene_info.pColorBlendState = &config.colorBlendInfo;
  pipeliene_info.pDynamicState = &config.dynamicStateInfo;

  pipeliene_info.layout = config.pipelineLayout;
  pipeliene_info.renderPass = config.renderPass;
  pipeliene_info.subpass = config.subpass;
  pipeliene_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipeliene_info.basePipelineIndex = -1;              // Optional

  if (vkCreateGraphicsPipelines(m_device.getDeviceHandle(), VK_NULL_HANDLE, 1,
                                &pipeliene_info, nullptr,
                                &m_graphics_pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }
}

} // namespace Render::Vulkan
