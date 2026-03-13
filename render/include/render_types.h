#pragma once

#include "backend_type.h"  // ← Core::BackendType
#include "glm/ext/vector_float4.hpp"
#include "resource_types.h"
#include "vertex_layout.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Render {

// Используем BackendType из Core
using Core::BackendType;

// --- Core Enums ---
enum class ImageLayout {
  UNDEFINED,
  COLOR_ATTACHMENT,
  DEPTH_STENCIL_ATTACHMENT,
  PRESENT_SRC,
  TRANSFER_DST,
  SHADER_READ_ONLY,
};

enum class LoadOp { DONT_CARE, LOAD, CLEAR };
enum class StoreOp { DONT_CARE, STORE };


enum class DescriptorType {
  SAMPLER,
  COMBINED_IMAGE_SAMPLER,
  SAMPLED_IMAGE,
  STORAGE_IMAGE,
  UNIFORM_BUFFER,
  STORAGE_BUFFER,
};

enum class IndexType { UINT16, UINT32 };

enum class ShaderStage {
  VERTEX = 0x00000001,
  FRAGMENT = 0x00000010,
  COMPUTE = 0x00000020,
};
using ShaderStageFlags = uint32_t;

enum class BufferUsage {
  TRANSFER_SRC = 0x0001,
  TRANSFER_DST = 0x0002,
  VERTEX_BUFFER = 0x0008,
  INDEX_BUFFER = 0x0010,
  UNIFORM_BUFFER = 0x0020,
};
using BufferUsageFlags = uint32_t;

enum class PrimitiveTopology {
  TRIANGLE_LIST,
  // Add others as needed
};

enum class PolygonMode {
  FILL,
  LINE,
  POINT,
};

enum class CullMode {
  NONE,
  FRONT,
  BACK,
  FRONT_AND_BACK,
};

enum class FrontFace {
  COUNTER_CLOCKWISE,
  CLOCKWISE,
};

// --- Resource Descriptors ---

struct BufferDesc {
  uint64_t size;
  BufferUsageFlags usage;
  bool is_host_visible = true;
  void *initial_data = nullptr;
  VertexLayout vertex_layout;
};

struct TextureDesc {
  uint32_t width;
  uint32_t height;
};

struct SamplerDesc {};

// --- Layout Descriptors ---

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
  Core::VertexLayout vertex_layout;
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

} // namespace Render
