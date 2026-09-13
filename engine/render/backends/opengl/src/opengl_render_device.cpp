#include "opengl_render_device.h"
#include "core/rid.h"
#include "opengl_buffer_objects.h" // For VAO, VBO, VBOLayout
#include "opengl_descriptor_set.h" // For UniformBuffer, OpenGLDescriptorSet
#include "opengl_gpu_storage.h"
#include "opengl_shader_module.h"
#include "opengl_shader_program.h" // Needed for ShaderProgram
#include "utils/debug_assert.h"

#include "core/render_types.h" // Needed for GraphicsPipelineDesc
#include "core/resource_types.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
namespace ssme::opengl {

OpenGLRenderDevice::OpenGLRenderDevice(OpenGLGpuStorageMT &storage)
    : m_storage(storage) {}

OpenGLRenderDevice::~OpenGLRenderDevice() {}

//------------------------------------------------------------------------
// ---------------------------- Buffer -----------------------------------
// -----------------------------------------------------------------------

RID OpenGLRenderDevice::createBuffer(const BufferDesc &desc, RID id) {
  debug_assert(desc.size > 0, "Buffer size cannot be zero");

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

    if (id.isNull()) {
      id = m_storage.add(std::move(ubo));
    } else {
      m_storage.store(id, std::move(ubo));
    }

    debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
    return id;
  } else if (desc.usage & static_cast<uint32_t>(BufferUsage::VERTEX_BUFFER)) {
    const auto stride = desc.layout.getStride();
    debug_assert(stride > 0, "Vertex Buffer must have a non-zero stride");
    // Create Vertex Buffer Object (VBO) with VAO
    uint64_t vertex_count = desc.size / stride;
    debug_assert(vertex_count > 0, "Vertex Buffer size cannot be empty");
    // 1. Create and initialize VBO
    auto vao = std::make_unique<VAO>();
    debug_assert(vao != nullptr, "Failed to allocate VAO");
    auto vbo = std::make_unique<VBO>();
    debug_assert(vbo != nullptr, "Failed to allocate VBO");

    vao->bind();
    vbo->init(desc.initial_data, desc.size);
    vao->addBuffer(*vbo, desc.layout, vertex_count);
    vbo->unbind();
    vao->unbind();

    if (id.isNull()) {
      id = m_storage.add(std::move(vao));
    } else {
      m_storage.store(id, std::move(vao));
    }
    debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
    return id;
  } else if (desc.usage & static_cast<uint32_t>(BufferUsage::INDEX_BUFFER)) {
    const auto index_size = desc.element_size;
    debug_assert(index_size > 0, "Index Buffer must have a non-zero stride");
    // Create Index Buffer Object (EBO)
    uint64_t index_count = desc.size / index_size;
    debug_assert(index_count > 0, "Index Buffer size cannot be empty");
    // 1. Create and initialize VBO
    auto ebo = std::make_unique<EBO>();
    debug_assert(ebo != nullptr, "Failed to allocate EBO");

    ebo->bind();
    ebo->init(desc.initial_data, index_count);
    ebo->unbind();

    if (id.isNull()) {
      id = m_storage.add(std::move(ebo));
    } else {
      m_storage.store(id, std::move(ebo));
    }
    debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
    return id;
  } else {
    debug_assert(false, "Unsupported buffer type");
    return RID::INVALID;
  }
}

void OpenGLRenderDevice::destroyBuffer(RID rid) {
  if (rid.isNull())
    return;

  // Try to remove as VAO first (vertex buffer)
  if (m_storage.remove<VAO>(rid))
    return;

  // Try to remove as UniformBuffer
  if (m_storage.remove<UniformBuffer>(rid))
    return;
}

//------------------------------------------------------------------------
// ---------------------------- Texture ----------------------------------
// -----------------------------------------------------------------------

RID OpenGLRenderDevice::createTexture(const TextureDesc &desc, RID id) {
  auto tex = std::make_unique<OpenGLTexture>(desc);

  if (id.isNull()) {
    id = m_storage.add(std::move(tex));
  } else {
    m_storage.store(id, std::move(tex));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void OpenGLRenderDevice::destroyTexture(RID id) {
  if (id.isNull())
    return;
  // Get buffer from storage
  auto *ds_layout = m_storage.get<OpenGLTexture>(id);
  if (ds_layout) {
    // Remove from storage (this will call OpenGLTexture destructor)
    m_storage.remove<OpenGLTexture>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Sampler ----------------------------------
// -----------------------------------------------------------------------

RID OpenGLRenderDevice::createSampler(const SamplerDesc &desc, RID id) {
  return {};
}

//------------------------------------------------------------------------
// ---------------------------- Descriptor layout ------------------------
// -----------------------------------------------------------------------

RID OpenGLRenderDevice::createDescriptorLayout(const DescriptorLayout &desc,
                                               RID id) {
  auto layout = std::make_unique<OpenGLDescriptorSetLayout>();

  for (const auto &binding : desc.bindings) {
    layout->addBinding(binding.binding, static_cast<uint32_t>(binding.type),
                       binding.stages, binding.count);
  }

  if (id.isNull()) {
    id = m_storage.add(std::move(layout));
  } else {
    m_storage.store(id, std::move(layout));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void OpenGLRenderDevice::destroyDescriptorLayout(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *ds_layout = m_storage.get<OpenGLDescriptorSetLayout>(id);
  if (ds_layout) {
    // Remove from storage (this will call OpenGLDescriptorSetLayout destructor)
    m_storage.remove<OpenGLDescriptorSetLayout>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Descriptor -------------------------------
// -----------------------------------------------------------------------

RID OpenGLRenderDevice::createDescriptor(const DescriptorDesc &desc, RID id) {
  // Get layout (for validation, optional)
  auto layout = m_storage.get<OpenGLDescriptorSetLayout>(desc.layout_id);
  if (!layout) {
    throw std::runtime_error("Invalid descriptor set layout RID in OpenGL");
  }

  // Create DescriptorSet
  auto desc_set = std::make_unique<OpenGLDescriptorSet>();

  // Add binding for each buffer
  for (size_t i = 0; i < desc.uniform_buffers.size(); ++i) {
    auto *ubo = m_storage.get<UniformBuffer>(desc.uniform_buffers[i]);
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

  for (size_t i = 0; i < desc.sampled_images.size(); ++i) {
    auto *texture = m_storage.get<OpenGLTexture>(desc.sampled_images[i]);
    if (!texture) {
      throw std::runtime_error("Invalid texture RID in createDescriptorSet");
    }
    desc_set->addBinding(0, texture->getTexture(), true);
  }

  if (id.isNull()) {
    id = m_storage.add(std::move(desc_set));
  } else {
    m_storage.store(id, std::move(desc_set));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void OpenGLRenderDevice::destroyDescriptor(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *ds_layout = m_storage.get<OpenGLDescriptorSet>(id);
  if (ds_layout) {
    // Remove from storage (this will call OpenGLDescriptorSet destructor)
    m_storage.remove<OpenGLDescriptorSet>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Pipeline layout --------------------------
// -----------------------------------------------------------------------

RID OpenGLRenderDevice::createPipelineLayout(const PipelineLayoutDesc &desc,
                                             RID id) {
  std::size_t hash = desc.hash();
  // 1. Check if a PSO with this name already exists
  RID existing_rid = m_storage.findPSOLayout(hash);
  if (existing_rid) {
    debug_assert(
        false, "External error in Resource manager. Atept to create Pipeline "
               "layout that already "
               "existed. Resource manager is not found by PipelimeParams HASH");
    // return existing_rid;
  }
  m_storage.registerPSOLayout(hash, id);

  return {};
}

void OpenGLRenderDevice::destroyPipelineLayout(RID id) {}
RID OpenGLRenderDevice::containsPipelineLayout(std::size_t hash) {
  RID existing_rid = m_storage.findPSOLayout(hash);
  if (existing_rid) {
    return existing_rid;
  }
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Pipeline ---------------------------------
// -----------------------------------------------------------------------

RID OpenGLRenderDevice::createGraphicsPipeline(const GraphicsPipelineDesc &desc,
                                               RID id) {
  std::size_t hash = desc.hash();
  // 1. Check if a PSO with this name already exists
  RID existing_rid = m_storage.findPSO(hash);
  if (existing_rid) {
    debug_assert(
        false,
        "External error in Resource manager. Atept to create PSO that already "
        "existed. Resource manager is not found by PipelimeParams HASH");
    // return existing_rid;
  }

  // --- If not found, create a new one ---
  auto vert_shader = m_storage.get<OpenGLShaderModule>(desc.vert_shader_module);
  auto frag_shader = m_storage.get<OpenGLShaderModule>(desc.frag_shader_module);

  if (!vert_shader || !frag_shader) {
    throw std::runtime_error(
        "Vertex and Fragment shader paths must be provided for OpenGL");
  }

  auto shader_program =
      std::make_unique<ShaderProgram>(*vert_shader, *frag_shader);

  if (id.isNull()) {
    id = m_storage.add(std::move(shader_program));
  } else {
    m_storage.store(id, std::move(shader_program));
  }
  m_storage.registerPSO(hash, id);

  return id;
}

void OpenGLRenderDevice::destroyGraphicsPipeline(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *pipeline = m_storage.get<ShaderProgram>(id);
  if (pipeline) {
    // Remove from storage (this will call ShaderProgram destructor)
    m_storage.remove<ShaderProgram>(id);
  }
}

RID OpenGLRenderDevice::containsGraphicsPipeline(std::size_t hash) {
  RID existing_rid = m_storage.findPSO(hash);
  if (existing_rid) {
    return existing_rid;
  }
  return RID::INVALID;
}

//------------------------------------------------------------------------
// ---------------------------- Shader -----------------------------------
// -----------------------------------------------------------------------

RID OpenGLRenderDevice::createShaderModule(const ShaderModuleDesc &desc,
                                           RID id) {
  GLenum stage = desc.stage == ssme::ShaderStage::VERTEX ? GL_VERTEX_SHADER
                                                         : GL_FRAGMENT_SHADER;
  auto path = "res/shaders/" + desc.file_path + ".glsl";
  // auto shader_module = std::make_unique<OpenGLShaderModule>(stage, path);
  auto shader_module = std::make_unique<OpenGLShaderModule>(stage, desc.code.glsl);
  if (id.isNull()) {
    id = m_storage.add(std::move(shader_module));
  } else {
    m_storage.store(id, std::move(shader_module));
  }
  debug_assert(id != RID::INVALID, "GpuStorage failed to assign a valid RID");
  return id;
}

void OpenGLRenderDevice::destroyShaderModule(RID id) {
  if (id.isNull())
    return;

  // Get buffer from storage
  auto *shader_module = m_storage.get<OpenGLShaderModule>(id);
  if (shader_module) {
    // Remove from storage (this will call OpenGLShaderModule destructor)
    m_storage.remove<OpenGLShaderModule>(id);
  }
}

//------------------------------------------------------------------------
// ---------------------------- Common -----------------------------------
// -----------------------------------------------------------------------

void OpenGLRenderDevice::updateBufferRaw(RID rid, size_t offset, size_t size,
                                         const void *data) {
  // OpenGL buffers are always host-visible, so we can update them directly
  auto *ubo = m_storage.get<UniformBuffer>(rid);
  if (!ubo) {
    throw std::runtime_error("Invalid buffer RID in updateBufferRaw");
  }
  ubo->update(offset, size, data);
}

} // namespace ssme::opengl
