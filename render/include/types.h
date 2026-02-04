#pragma once

#include <cstdint>
#include "rid.h"

namespace Render {

// --- Core Types ---

using RID = Core::RID;

// --- Enums ---

enum class IndexType {
    UINT16,
    UINT32
};

enum class ShaderStage {
    VERTEX = 0x00000001,
    FRAGMENT = 0x00000010,
    COMPUTE = 0x00000020,
};
using ShaderStageFlags = uint32_t;


// --- Resource Descriptors ---

struct BufferDesc {
    uint64_t size;
};

struct TextureDesc {
    uint32_t width;
    uint32_t height;
};

struct SamplerDesc {};
struct GraphicsPipelineDesc {};


// --- Command Structs ---

struct Viewport {
    float x, y, width, height, minDepth, maxDepth;
};

struct Rect {
    int32_t x, y;
    uint32_t width, height;
};

struct BufferCopy {
    uint64_t srcOffset;
    uint64_t dstOffset;
    uint64_t size;
};

struct BufferImageCopy {
    uint64_t bufferOffset;
};

struct BarrierInfo {};
struct RenderingInfo {};

} // namespace Render
