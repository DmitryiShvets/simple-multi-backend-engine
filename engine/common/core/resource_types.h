#pragma once

#include "rid.h"
#include "render_types.h"
#include "vertex_layout.h"
#include <glm/vec4.hpp>
#include "uniform_layout.h"
#include <cstddef>

namespace ssme {

// ==================== Supported resource types ====================

enum class ResourceId : size_t {
  // for GPU
  BUFFER = 0,
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
  MATERIAL_TEMPLATE,
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
  DESCRIPTOR_SET,       // Allocated descriptor set
  DESCRIPTOR_POOL,      // Descriptor pool that owns descriptor sets
  PIPELINE_LAYOUT,
  MATERIAL_TEMPLATE,
};
// ==================== Resource Descriptors ====================

struct BufferDesc {
  uint64_t size;
  BufferUsageFlags usage;
  bool is_host_visible = true;
  void *initial_data = nullptr;
  VertexLayout layout;
};

struct TextureDesc {
  uint32_t width;
  uint32_t height;
};

struct SamplerDesc {
  // TODO: FILL
};

// ==================== Layout Descriptors ====================

struct DescriptorBindingDesc {
  uint32_t binding;
  DescriptorType type;
  ShaderStageFlags stages;
  uint32_t count = 1;
};

struct DescriptorSetLayoutDesc {
  std::vector<DescriptorBindingDesc> bindings;
};

struct PushConstantRange {
  ShaderStageFlags stages;
  uint32_t offset;
  uint32_t size;
};

struct PipelineLayoutDesc {
  std::vector<RID> descriptor_set_layouts;
  std::vector<PushConstantRange> push_constant_ranges;
};

// --- Graphics Pipeline Descriptors ---



struct ShaderModuleDesc {
  std::string file_path;
  ShaderStage stage;
};

struct RasterizationStateDesc {
  PolygonMode polygonMode = PolygonMode::FILL;
  CullMode cullMode = CullMode::BACK;
  FrontFace frontFace = FrontFace::COUNTER_CLOCKWISE;
  bool depthBiasEnable = false;
};

struct DepthStencilStateDesc {
  bool depthTestEnable = true;
  bool depthWriteEnable = true;
};

struct GraphicsPipelineDesc {
  std::string name;
  RID pipeline_layout_rid; // Now takes a pre-created layout
  std::vector<ShaderModuleDesc> shader_modules;
  VertexLayout vertex_layout;
  PrimitiveTopology primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
  RasterizationStateDesc rasterization_state;
  DepthStencilStateDesc depth_stencil_state;
};

// --- Command Structs ---

struct Viewport {
  float x, y, width, height, minDepth, maxDepth;
};

struct Rect {
  int32_t x, y;
  uint32_t width, height;
};

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


// Описание материала для конкретного бэкенда
struct PipelineDesc {
  std::vector<DescriptorSetLayoutDesc> ds_layouts_desc;  // Array of descriptor set layouts (Set 0, Set 1, etc.)
  PipelineLayoutDesc pl_layout_desc;
  GraphicsPipelineDesc pl_desc;
};

// Полный материал с настройками для обоих бэкендов
struct PipelineConfig {
  std::string name;
  PipelineDesc desc;
  UniformLayout uniform_layout;  // Layout для material uniforms (Set 1)
  UniformLayout object_uniform_layout;  // Layout для object uniforms (Set 2)

  static PipelineConfig create(
      const std::string &name,
      const std::function<void(PipelineDesc &)> &configure_vk = nullptr);
};


enum class ResourceState { UNDEFINED, TRANSFER_DST, PRESENT_SRC };

struct Material {
  std::string name;
  struct RenderData {
    RID pipeline;
    RID uniforms_buf;   // Material-level uniform buffer RID (albedo, roughness, etc.)
    RID uniforms_ds;    // Descriptor set for material-level resources
    RID object_uniform_ds_layout; // Descriptor set layout for object uniforms (for creating per-object DS)
  } render_data;
};

} // namespace ssme
