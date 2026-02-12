#include "opengl_device.h"
#include "common_utils.h"
#include "opengl_resource_manager.h"
#include "opengl_shader_program.h" // Needed for ShaderProgram
#include "render_types.h"          // Needed for GraphicsPipelineDesc
#include <stdexcept>
#include "opengl_buffer_objects.h" // For VAO, VBO, VBOLayout
#include "vertex.h" // For sizeof(Vertex)

namespace Render::OpenGL {

OpenGLDevice::OpenGLDevice(OpenglResourceManager &resource_manager)
    : m_resource_manager(resource_manager) {}
OpenGLDevice::~OpenGLDevice() {}

RID OpenGLDevice::createBuffer(const BufferDesc &desc) {
  if (!desc.initial_data || desc.size == 0) {
    return {};
  }

  // 1. Define the layout of the buffer
  VBOLayout layout;
  layout.addLayoutElement(3, GL_FLOAT, GL_FALSE);   // Location 0: Position
  layout.addLayoutElement(3, GL_FLOAT, GL_FALSE);   // Location 1: Color
  uint64_t vertex_count = desc.size / sizeof(Vertex);

  // 1. Create and initialize VBO
  auto vao = std::make_unique<VAO>();
  auto vbo = std::make_unique<VBO>();
  vao->bind();
  vbo->init(desc.initial_data, desc.size);
  vao->addBuffer(*vbo, layout, vertex_count);
  vbo->unbind();
  vao->unbind();

  return m_resource_manager.add(std::move(vao));
}


RID OpenGLDevice::createTexture(const TextureDesc &desc) { return {}; }
RID OpenGLDevice::createSampler(const SamplerDesc &desc) { return {}; }
RID OpenGLDevice::createDescriptorSetLayout(
    const DescriptorSetLayoutDesc &desc) {
  return {};
}
RID OpenGLDevice::createPipelineLayout(const PipelineLayoutDesc &desc) {
  return {};
}
RID OpenGLDevice::createGraphicsPipeline(const GraphicsPipelineDesc &desc) {
  // 1. Check if a PSO with this name already exists
  RID existing_rid = m_resource_manager.findPSO(desc.name);
  if (existing_rid) {
    return existing_rid;
  }

  // --- If not found, create a new one ---
  std::string vert_path, frag_path;
  for (const auto &shader_module : desc.shader_modules) {
    if (shader_module.stage == ShaderStage::VERTEX) {
      vert_path = shader_module.file_path;
    } else if (shader_module.stage == ShaderStage::FRAGMENT) {
      frag_path = shader_module.file_path;
    }
  }
  if (vert_path.empty() || frag_path.empty()) {
    throw std::runtime_error(
        "Vertex and Fragment shader paths must be provided for OpenGL");
  }

  auto shader_program =
      std::make_unique<ShaderProgram>(CUtils::readFile(vert_path), CUtils::readFile(frag_path));

  RID new_rid = m_resource_manager.add(std::move(shader_program));
  m_resource_manager.registerPSO(desc.name, new_rid);

  return new_rid;
}
void OpenGLDevice::free(RID rid) {}

} // namespace Render::OpenGL
