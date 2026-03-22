#pragma once

#include <cstdint>

namespace ssme {
// ==================== Core Enums ====================

using ShaderStageFlags = uint32_t;
using BufferUsageFlags = uint32_t;

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

enum class BufferUsage {
  TRANSFER_SRC = 0x0001,
  TRANSFER_DST = 0x0002,
  VERTEX_BUFFER = 0x0008,
  INDEX_BUFFER = 0x0010,
  UNIFORM_BUFFER = 0x0020,
  STORAGE_BUFFER = 0x0040,
  INDIRECT_BUFFER = 0x0080,
};

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

enum class Format {
  UNDEFINED,
  R32G32B32_SFLOAT,
  R32G32_SFLOAT,
  // Add other formats as needed
};


struct Viewport {
  float x, y, width, height, minDepth, maxDepth;
};

struct Rect {
  int32_t x, y;
  uint32_t width, height;
};
} // namespace ssme
