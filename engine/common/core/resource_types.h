#pragma once

#include "render_types.h"
#include "rid.h"
#include "uniform_layout.h"
#include "utils/hash_utils.h"
#include "vertex_layout.h"
#include <cstddef>
#include <glm/vec4.hpp>
#include <map>
#include <memory>

namespace ssme {

// ==================== Supported resource types ====================

enum class ResourceId : size_t {
  // for GPU
  BUFFER = 0,
  UNIFORM_BUFFER,
  TEXTURE,
  IMAGE,
  IMAGE_VIEW,
  SHADER,
  SAMPLER,
  PIPELINE,
  PIPELINE_LAYOUT,
  DESCRIPTOR_SET,
  DESCRIPTOR_SET_LAYOUT,
  DESCRIPTOR_POOL,
  // for USERS
  MATERIAL,
  MESH,
  SKELETON,
  ANIMATION,
  SOUND,
  FONT,
  CONFIG,
};

// TODO: REMOVE legacy
enum class ResourceType {
  UNDEFINED,
  SWAP_CHAIN,
  TEXTURE,
  IMAGE,
  IMAGE_VIEW,
  BUFFER,
  PIPELINE,
  DESCRIPTOR_SET_LAYOUT,
  DESCRIPTOR_SET,  // Allocated descriptor set
  DESCRIPTOR_POOL, // Descriptor pool that owns descriptor sets
  PIPELINE_LAYOUT,
  MATERIAL_TEMPLATE,
};
// ==================== Resource Descriptors ====================

struct AABB {
  glm::vec3 min{0.0f};
  glm::vec3 max{0.0f};
};

struct BufferDesc {
  uint64_t size = 0;
  uint64_t element_size = 0;
  BufferUsageFlags usage = 0;
  bool is_host_visible = true;
  void *initial_data = nullptr;
  VertexLayout layout;
};

/**
 * @brief Mesh resource requirements
 */
struct MeshDesc {
  std::vector<uint8_t> vertices;
  std::vector<uint32_t> indices;
  VertexLayout layout;
  AABB bounds;
};

struct SamplerDesc {
  Filter min_filter = Filter::LINEAR;
  Filter mag_filter = Filter::LINEAR;
  Wrap wrap_s = Wrap::REPEAT;
  Wrap wrap_t = Wrap::REPEAT;
};

/**
 * @brief Texture resource requirements
 */
struct TextureDesc {
  uint32_t width = 0;
  uint32_t height = 0;
  Format format = Format::R8G8B8A8_UNORM;
  ImageUsage usage = ImageUsage::SHADER_READ;
  // Sampler params
  SamplerDesc sampler;

  bool generate_mips = true;
  std::string source_path = "";  // File path (if loading from disk)
  std::vector<uint8_t> raw_data; // Raw pixels (if procedural)
};



// ==================== Shader Descriptors ====================

// Information about specific Binding (slot)
// Created in shader via reflection. Previously created manually in Pipeline
// config
struct Binding {
  uint32_t set;
  uint32_t binding;
  std::string name;
  std::string type_name;
  DescriptorType type;
  uint32_t count = 1; // purpose unclear.
  ShaderStageFlags stages;

  bool operator==(const Binding &other) const {
    return set == other.set && binding == other.binding && name == other.name &&
           type == other.type && count == other.count && stages == other.stages;
  }

  std::size_t hash() const {
    std::size_t h = 0;
    hash_combine(h, set, binding, name, static_cast<uint32_t>(type), count,
                 stages);
    return h;
  }
};

// Created in shader via reflection. Previously created manually
struct DescriptorLayout {
  std::vector<Binding> bindings;

  bool operator==(const DescriptorLayout &other) const {
    return bindings == other.bindings;
  }

  std::size_t hash() const {
    std::size_t h = 0;
    for (const auto &b : bindings) {
      hash_combine(h, b.hash());
    }
    return h;
  }

  std::string uuid() const { return std::format("ds_layout{:016x}", hash()); }
};

struct PushConstantRange {
  ShaderStageFlags stages;
  uint32_t offset;
  uint32_t size;

  bool operator==(const PushConstantRange &other) const {
    return stages == other.stages && offset == other.offset &&
           size == other.size;
  }

  std::size_t hash() const {
    std::size_t h = 0;
    hash_combine(h, static_cast<uint32_t>(stages), offset, size);
    return h;
  }
};

/**
 * @brief Descriptor Set description
 *
 * Contains references to buffers/textures for binding
 */
struct DescriptorDesc {
  RID layout_id;                    // layout ID (must be created)
  std::vector<RID> uniform_buffers; // Uniform buffer RIDs
  std::vector<RID> storage_buffers; // Storage buffer RIDs
  std::vector<RID> sampled_images;  // Texture RIDs
  std::vector<RID> samplers;        // Sampler RIDs

  bool operator==(const DescriptorDesc &other) const {
    return layout_id == other.layout_id &&
           uniform_buffers == other.uniform_buffers &&
           storage_buffers == other.storage_buffers &&
           sampled_images == other.sampled_images && samplers == other.samplers;
  }

  std::size_t hash() const {
    std::size_t h = 0;
    hash_combine(h, layout_id);
    for (const auto &buf : uniform_buffers) {
      hash_combine(h, buf);
    }
    for (const auto &buf : storage_buffers) {
      hash_combine(h, buf);
    }
    for (const auto &img : sampled_images) {
      hash_combine(h, img);
    }
    for (const auto &smp : samplers) {
      hash_combine(h, smp);
    }
    return h;
  }

  std::string uuid() const { return std::format("ds_{:016x}", hash()); }
};

struct ShaderReflectionData {
  std::vector<VertexInputRequirement> vertex_requirements;
  std::map<std::string, PushConstantRange> push_constants;
  std::map<std::string, std::shared_ptr<UniformLayout>> push_constants_layouts;
  std::map<std::string, std::shared_ptr<UniformLayout>> binding_layouts;
  std::map<uint32_t, DescriptorLayout> ds_layouts;
  uint32_t descriptor_set_count = 0;
  uint32_t required_components = 0;
};

struct ShaderCodeDesc {
  std::vector<char> spirv;
  std::vector<char> glsl;
  std::vector<char> dxil;
};

struct ShaderModuleDesc {
  std::string file_path;
  ShaderStage stage;
  ShaderCodeDesc code;
  ShaderReflectionData reflection;
};

// ==================== Pipeline Descriptors ====================

struct PipelineLayoutDesc {
  std::vector<RID> descriptor_layouts;
  std::vector<PushConstantRange> push_constant_ranges;

  bool operator==(const PipelineLayoutDesc &other) const {
    return descriptor_layouts == other.descriptor_layouts &&
           push_constant_ranges == other.push_constant_ranges;
  }

  std::size_t hash() const {
    std::size_t h = 0;
    for (const auto &layout : descriptor_layouts) {
      hash_combine(h, layout);
    }
    for (const auto &p : push_constant_ranges) {
      hash_combine(h, p.hash());
    }
    return h;
  }
};

struct RasterizationStateDesc {
  PolygonMode polygonMode = PolygonMode::FILL;
  CullMode cullMode = CullMode::BACK;
  FrontFace frontFace = FrontFace::COUNTER_CLOCKWISE;
  bool depthBiasEnable = false;

  bool operator==(const RasterizationStateDesc &other) const {
    return polygonMode == other.polygonMode && cullMode == other.cullMode &&
           frontFace == other.frontFace &&
           depthBiasEnable == other.depthBiasEnable;
  }

  std::size_t hash() const {
    std::size_t h = 0;
    hash_combine(h, static_cast<uint32_t>(polygonMode),
                 static_cast<uint32_t>(cullMode),
                 static_cast<uint32_t>(frontFace), depthBiasEnable);

    return h;
  }
};

struct DepthStencilStateDesc {
  bool depthTestEnable = false;
  bool depthWriteEnable = false;

  bool operator==(const DepthStencilStateDesc &other) const {
    return depthTestEnable == other.depthTestEnable &&
           depthWriteEnable == other.depthWriteEnable;
  }

  std::size_t hash() const {
    std::size_t h = 0;
    hash_combine(h, depthTestEnable, depthWriteEnable);
    return h;
  }
};

struct GraphicsPipelineDesc {
  RID pl_layout_id;           // id of layout. must be already created
  VertexLayout vertex_layout; // from mesh
  RID vert_shader_module; // id of vertex shader module. must be already created
  RID frag_shader_module; // id of fragment shader module. must be already
                          // created
  DepthStencilStateDesc depth_stencil_state;
  Format color_attachment_format = Format::R8G8B8A8_SRGB;
  Format depth_attachment_format = Format::D32F;

  std::size_t hash() const {
    std::size_t h = 0;
    hash_combine(h, vertex_layout.hash(), vert_shader_module,
                 frag_shader_module, depth_stencil_state.hash(),
                 static_cast<uint32_t>(depth_attachment_format),
                 static_cast<uint32_t>(color_attachment_format));
    return h;
  }

  bool operator==(const GraphicsPipelineDesc &other) const {
    return vertex_layout == other.vertex_layout &&
           vert_shader_module == other.vert_shader_module &&
           frag_shader_module == other.frag_shader_module &&
           depth_stencil_state == other.depth_stencil_state &&
           color_attachment_format == other.color_attachment_format &&
           depth_attachment_format == other.depth_attachment_format;
  }
};

// --- Command Structs ---

struct ColorAttachmentInfo {
  RID texture;
  LoadOp load_op = LoadOp::DONT_CARE;
  StoreOp store_op = StoreOp::STORE;
  glm::vec4 clear_value;
  ImageLayout initial_layout = ImageLayout::UNDEFINED;
  ImageLayout final_layout = ImageLayout::PRESENT_SRC;
};

struct DepthAttachmentInfo {
  RID texture;
  LoadOp load_op = LoadOp::CLEAR;
  StoreOp store_op = StoreOp::DONT_CARE;
  float clear_value = 1.0f;
};

struct RenderingInfo {
  Rect render_area;
  std::vector<ColorAttachmentInfo> color_attachments;
  DepthAttachmentInfo depth_attachment;
};

struct BufferCopy {
  uint64_t srcOffset;
  uint64_t dstOffset;
  uint64_t size;
};

struct BufferImageCopy {
  uint64_t bufferOffset;
};

struct ImageBarrierDesc {
  RID image;
  ImageLayout old_layout;
  ImageLayout new_layout;
};

struct BarrierInfo {
  std::vector<ImageBarrierDesc> image_barriers;
};

} // namespace ssme
