
#pragma once
#include "command_list.h"
#include "core/render_types.h"
#include "core/resource_types.h"
#include "core/rid.h"
#include "resource_handle.h"
#include "resources/texture.h"
#include <string>

namespace ssme {

using ResourceIndex = uint32_t; // readable alias for resource array indices
using PassIndex = uint32_t;     // readable alias for pass array indices
using BlockIndex = uint32_t;    // index into the physical-block free list
using ExecuteCallback = std::function<void(CommandList &cmd)>;

enum class ResourceFormat {
  RGBA8,
  RGBA16F,
  R8,
  D32F,
};

struct ResourceDesc {
  uint32_t width;
  uint32_t height;
  ResourceFormat format = ResourceFormat::RGBA8;
};

// Lightweight handle, index into FrameGraph's resource array, no GPU memory
// involved.
struct ResourceView {
  ResourceIndex index = UINT32_MAX;
  bool isValid() const { return index != UINT32_MAX; }
};

struct ResourceVersion {
  PassIndex writer_pass =
      UINT32_MAX; // Each read() links to the current version's writer →
                  // automatic dependency edge.
  std::vector<PassIndex>
      reader_passes; // Each write() to a resource creates a new version.
  bool hasWriter() const { return writer_pass != UINT32_MAX; }
};

enum class ResourceState : uint32_t {
  UNDEFINED = 0,
  COLOR_ATTACHMENT = 1 << 0,
  DEPTH_ATTACHMENT = 1 << 1,
  SHADER_READ = 1 << 2,
  UNORDERED_ACCESS = 1 << 3,
  PRESENT = 1 << 4,
  TRANSFER_DST = 1 << 5,
  TRANSFER_SRC = 1 << 6,
};

struct ResourceEntry {
  ResourceDesc desc;
  std::vector<ResourceVersion> versions; // version 0, 1, 2...
  bool imported = false; // imported = externally owned (e.g. swapchain)
  ResourceState current_state = ResourceState::UNDEFINED;
};

// Base Barrier already defined in v2, v3 adds aliasing context.
struct ResourceBarrier {
  ResourceIndex resource_index;
  ResourceState old_state;
  ResourceState new_state;
  bool is_aliasing = false; // aliasing barrier (block changes occupant)
  ResourceIndex alias_before = UINT32_MAX; // resource being evicted
};

// A physical memory slot, multiple virtual resources can reuse it if their
// lifetimes don't overlap.
struct PhysicalBlock {
  uint32_t size_bytes = 0;   // block size (aligned)
  PassIndex avail_after = 0; // free after this sorted pass
};
// Per-resource lifetime in sorted-pass indices, drives aliasing decisions.
struct Lifetime {
  PassIndex first_use =
      UINT32_MAX;           // first sorted pass that touches this resource
  PassIndex last_use = 0;   // last sorted pass that touches this resource
  bool is_transient = true; // false for imported resources (externally owned)
};

// Minimum placement alignment for aliased heap resources (real APIs enforce
// similar, e.g. 64 KB).
static constexpr uint32_t kPlacementAlignment = 65536; // 64 KB
inline uint32_t AlignUp(uint32_t value, uint32_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}
inline uint32_t BytesPerPixel(ResourceFormat fmt) {
  switch (fmt) {
  case ResourceFormat::R8:
    return 1;
  case ResourceFormat::RGBA8:
    return 4;
  case ResourceFormat::D32F:
    return 4;
  case ResourceFormat::RGBA16F:
    return 8;
  default:
    return 4;
  }
}

// Aligned allocation size, real drivers add row padding/tiling; we approximate
// with a round-up.
inline uint32_t AllocSize(const ResourceDesc &desc) {
  uint32_t raw = desc.width * desc.height * BytesPerPixel(desc.format);
  return AlignUp(raw, kPlacementAlignment);
}

struct CompiledPlan {
  std::vector<PassIndex> sorted;   // topological execution order
  std::vector<BlockIndex> mapping; // mapping[ResourceIndex] → physical block

  std::vector<std::vector<ResourceBarrier>>
      barriers; // barriers[orderIdx] → pre-pass transitions
};

// ── Внутренняя нода ресурса ──
struct TransientResource {
  std::string name;
  ResourceView view;
  ResourceHandle<Texture> handle; // выделен при compile()
  // slot management
  int producer_slot = -1;          // index слота, который produce
  std::vector<int> consumer_slots; // слоты, которые consume
  // transient management (заполняются после flatten)
  int first_pass = -1; // sorted_index первого write
  int last_pass = -1;  // sorted_index последнего read
  bool imported = false;
};

struct PassNode {
  std::vector<ResourceView> reads;
  std::vector<ResourceView> writes;
  std::vector<ResourceView> read_writes; // UAV (explicit)
  std::vector<PassIndex> depends_on;
  std::vector<PassIndex> successors; // passes that depend on this one
  uint32_t in_degree = 0;            // incoming edge count (Kahn's)
  bool alive = false;                // survives the cull?
  PassIndex index = UINT32_MAX;
};

} // namespace ssme
