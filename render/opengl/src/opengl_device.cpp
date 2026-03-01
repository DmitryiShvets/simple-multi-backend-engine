#include "opengl_device.h"
#include "logger.h"
#include "opengl_buffer_objects.h" // For VAO, VBO, VBOLayout
#include "opengl_descriptor_set.h" // For UniformBuffer, OpenGLDescriptorSet
#include "opengl_resource_manager.h"
#include "opengl_shader_program.h" // Needed for ShaderProgram

#include "common_utils.h"
#include "pipeline_config_registry.h"
#include "render_types.h" // Needed for GraphicsPipelineDesc
#include "resource_types.h"
#include "vertex.h" // For sizeof(Vertex)

#include <memory>
#include <stdexcept>
namespace Render::OpenGL {

OpenGLDevice::OpenGLDevice(OpenglResourceManager &resource_manager,
                           PipelineConfigRegistry &pl_registry)
    : m_resource_manager(resource_manager), m_pl_registry(pl_registry) {}
OpenGLDevice::~OpenGLDevice() {}

RID OpenGLDevice::createBuffer(const BufferDesc &desc) {
  if (desc.size == 0) {
    Logger::error_log("Cannot create buffer with zero size");
    return RID::INVALID;
  }

  // Create different buffer types based on usage
  if (desc.usage & static_cast<uint32_t>(BufferUsage::UNIFORM_BUFFER)) {
    // Create Uniform Buffer Object (UBO)
    // Use temporary zero-initialized data if no initial data provided
    std::vector<uint8_t> zero_data;
    const void *initial_data = desc.initial_data;
    if (!initial_data) {
      zero_data.resize(desc.size, 0);
      initial_data = zero_data.data();
    }
    auto ubo = std::make_unique<UniformBuffer>(desc.size, initial_data);
    return m_resource_manager.add(std::move(ubo));
  } else if (desc.usage & static_cast<uint32_t>(BufferUsage::VERTEX_BUFFER)) {
    const auto stride = desc.vertex_layout.getStride();
    if (stride == 0) {
      Logger::error_log("Cannot create buffer with zero stride");
      return RID::INVALID;
    }
    // Create Vertex Buffer Object (VBO) with VAO
    uint64_t vertex_count = desc.size / stride;

    // 1. Create and initialize VBO
    auto vao = std::make_unique<VAO>();
    auto vbo = std::make_unique<VBO>();
    vao->bind();
    vbo->init(desc.initial_data, desc.size);
    vao->addBuffer(*vbo, desc.vertex_layout, vertex_count);
    vbo->unbind();
    vao->unbind();

    return m_resource_manager.add(std::move(vao));
  } else {
    Logger::error_log("Unsupported buffer type");
    return RID::INVALID;
  }
}

RID OpenGLDevice::createTexture(const TextureDesc &desc) { return {}; }
RID OpenGLDevice::createSampler(const SamplerDesc &desc) { return {}; }
RID OpenGLDevice::createDescriptorSetLayout(
    const DescriptorSetLayoutDesc &desc) {
  auto layout = std::make_unique<OpenGLDescriptorSetLayout>();

  for (const auto &binding : desc.bindings) {
    layout->addBinding(binding.binding, static_cast<uint32_t>(binding.type),
                       binding.stages, binding.count);
  }

  return m_resource_manager.add(std::move(layout));
}

RID OpenGLDevice::createDescriptorSet(RID layout_rid,
                                      const std::vector<RID> &buffer_rids) {
  // Get layout (for validation, optional)
  auto layout =
      m_resource_manager.get_ptr<OpenGLDescriptorSetLayout>(layout_rid);
  if (!layout) {
    throw std::runtime_error("Invalid descriptor set layout RID in OpenGL");
  }

  // Create DescriptorSet
  auto desc_set = std::make_unique<OpenGLDescriptorSet>();

  // Add binding for each buffer
  for (size_t i = 0; i < buffer_rids.size(); ++i) {
    auto *ubo = m_resource_manager.get_ptr<UniformBuffer>(buffer_rids[i]);
    if (!ubo) {
      throw std::runtime_error("Invalid buffer RID in createDescriptorSet");
    }

    // Get binding point from layout (or use index)
    uint32_t binding_point = static_cast<uint32_t>(i);
    if (i < layout->getBindingCount()) {
      binding_point = layout->getBinding(i).binding;
    }

    desc_set->addBinding(binding_point, ubo->getHandle());
  }

  return m_resource_manager.add(std::move(desc_set));
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

  auto shader_program = std::make_unique<ShaderProgram>(
      CUtils::readFile(vert_path), CUtils::readFile(frag_path));

  RID new_rid = m_resource_manager.add(std::move(shader_program));
  m_resource_manager.registerPSO(desc.name, new_rid);

  return new_rid;
}

RID OpenGLDevice::createPipeline(const PipelineDesc &desc) {
  // TODO: FIX IT
  return createGraphicsPipeline(desc.pl_desc);
}

RID OpenGLDevice::createMaterial(const std::string &material_name,
                                 const UniformSet &material_uniforms) {
  // 1. Check if a material with this name already exists
  RID material_rid = m_resource_manager.findMaterial(material_name);
  if (material_rid) {
    // Material exists, just update uniforms
    auto *mat = m_resource_manager.get_ptr<Material>(material_rid);
    if (mat && mat->render_data.uniforms_buf) {
      // Get layout from registry
      auto *config = m_pl_registry.getByName(material_name);
      if (config) {
        // Pack uniforms and update buffer
        std::vector<uint8_t> packed =
            config->uniform_layout.pack(material_uniforms);
        updateBufferRaw(mat->render_data.uniforms_buf, 0, packed.size(),
                        packed.data());
      }
    }
    return material_rid;
  }

  // 2. Get material config from registry
  const PipelineConfig *config = m_pl_registry.getByName(material_name);
  if (!config) {
    throw std::runtime_error("Unknown material type: " + material_name);
  }
  // 3. Create pipeline
  RID pipeline_rid = createGraphicsPipeline(config->desc.pl_desc);
  // 4. Create material uniform buffer using layout
  const auto &layout = config->uniform_layout;
  std::vector<uint8_t> packed_uniforms = layout.pack(material_uniforms);

  RID mat_uniform_buffer = createBuffer(
      BufferDesc{.size = layout.getTotalSize(),
                 .usage = static_cast<uint32_t>(BufferUsage::UNIFORM_BUFFER),
                 .is_host_visible = true,
                 .initial_data = packed_uniforms.data()});
  // 5. Create descriptor set layout for material using config's binding
  // OpenGL has global binding space, so binding from config must match shader
  DescriptorSetLayoutDesc ds_layout_desc;
  // Get the binding from the pipeline config (should be 1 for OpenGL
  // MaterialUBO)
  const auto &mat_binding = config->desc.ds_layouts_desc[1].bindings[0];
  ds_layout_desc.bindings.push_back(mat_binding);
  RID mat_ds_layout = createDescriptorSetLayout(ds_layout_desc);
  // 6. Create descriptor set for material
  RID mat_desc_set = createDescriptorSet(mat_ds_layout, {mat_uniform_buffer});

  // 7. Create descriptor set layout for object uniforms (if used by material)
  RID obj_ds_layout = RID::INVALID;
  if (config->desc.ds_layouts_desc.size() > 2 &&
      !config->object_uniform_layout.getVariables().empty()) {
    DescriptorSetLayoutDesc obj_ds_layout_desc;
    const auto &obj_binding = config->desc.ds_layouts_desc[2].bindings[0];
    obj_ds_layout_desc.bindings.push_back(obj_binding);
    obj_ds_layout = createDescriptorSetLayout(obj_ds_layout_desc);
  }

  // 8. Create material template
  material_rid = m_resource_manager.add(std::make_unique<Material>(
      Material{.name = material_name,
               .render_data = {
                   .pipeline = pipeline_rid,
                   .uniforms_buf = mat_uniform_buffer,
                   .uniforms_ds = mat_desc_set,
                   .object_uniform_ds_layout = obj_ds_layout,
               }}));
  return material_rid;
}

const PipelineConfig *
OpenGLDevice::getPipelineConfig(const std::string &material_name) const {
  return m_pl_registry.getByName(material_name);
}

Material *OpenGLDevice::getMaterial(RID material_rid) {
  return m_resource_manager.get_ptr<Material>(material_rid);
}

void OpenGLDevice::free(RID rid) {}

void OpenGLDevice::updateBufferRaw(RID rid, size_t offset, size_t size,
                                   const void *data) {
  // OpenGL buffers are always host-visible, so we can update them directly
  auto *ubo = m_resource_manager.get_ptr<UniformBuffer>(rid);
  if (!ubo) {
    throw std::runtime_error("Invalid buffer RID in updateBufferRaw");
  }
  ubo->update(offset, size, data);
}

} // namespace Render::OpenGL
