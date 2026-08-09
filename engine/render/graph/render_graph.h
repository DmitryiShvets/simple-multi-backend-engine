#pragma once
#include "render_gtaph_utils.h"
#include "render_pass.h"
#include "render_slot.h"
#include "utils/debug_assert.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ssme {
class TransientPool;

// A graph structure that defines all the passes for a single frame
class RenderGraph {
public:
  RenderGraph();
  ~RenderGraph();

  // Move-only semantics
  RenderGraph(const RenderGraph &) = delete;
  RenderGraph &operator=(const RenderGraph &) = delete;
  RenderGraph(RenderGraph &&) = default;
  RenderGraph &operator=(RenderGraph &&) = default;

  // ── Slot management ──
  template <typename T> T &addSlot() {
    auto slot = std::make_unique<T>();
    for (auto &existing : m_slots)
      debug_assert(existing->name() != slot->name(), "addSlot: duplicate slot");
    m_slots.push_back(std::move(slot));
    return *static_cast<T *>(m_slots.back().get());
  }

  // ── Resource registration ──
  // Create a virtual resource -- returns a handle, not GPU memory.
  ResourceView addResource(const std::string &name, const ResourceDesc &desc);
  ResourceView getResource(const std::string &name) const;
  // Import an external resource (e.g. swapchain backbuffer).
  // Barriers are tracked, but the graph does not own its memory.
  ResourceView
  importResource(const std::string &name, const ResourceDesc &desc,
                 ResourceState initialState = ResourceState::UNDEFINED);
  // Declarative API for specifying resource dependencies
  void read(PassIndex pass, ResourceView resource);
  void read(PassIndex pass, ResourceView view, ResourceState state);
  void write(PassIndex pass, ResourceView resource);
  void write(PassIndex pass, ResourceView view, ResourceState state);
  void readWrite(PassIndex pass, ResourceView resource);
  RID ridOf(ResourceView view) const { return m_rid_by_view[view.index]; }
  // Adds a new pass to the graph, returning a reference to it for configuration
  RenderPass &addPass(const std::string &name);
  void bindImport(ResourceView view, RID rid);

  // ── Compilation ──
  void build();
  CompiledPlan compile(TransientPool *pool);

  // ── Runtime ──
  void execute(CommandList &cmd, const CompiledPlan &plan,
               const Rect &render_area);
  void reset();
  // Reset all transient (non-imported) resources back to COMMON so the next
  // frame's first transition (UNDEFINED→X) matches D3D12's persistent state.
  void resetTransientToCommon(CommandList &cmd);

private:
  // new
  void buildEdges();
  std::vector<PassIndex> topoSort();
  void cull(const std::vector<PassIndex> &sorted);
  std::vector<std::vector<ResourceBarrier>>
  computeBarriers(const std::vector<PassIndex> &sorted,
                  const std::vector<BlockIndex> &mapping);
  void emitBarriers(CommandList &cmd,
                    const std::vector<ResourceBarrier> &barriers,
                    const std::vector<RID> &rid_by_view);
  std::vector<Lifetime> scanLifetimes(const std::vector<PassIndex> &sorted);
  std::vector<BlockIndex>
  aliasResources(const std::vector<Lifetime> &lifetimes);
  ResourceState stateForUsage(PassIndex passIdx, ResourceView view,
                              bool isWrite) const;
  void buildCompiledPasses(const CompiledPlan &plan,
                           const std::vector<RID> &rid_by_view);
  bool isFirstWriter(PassIndex passIdx, ResourceView view) const;
  RenderingInfo buildRenderingInfo(const PassNode &pass,
                                   const std::vector<RID> &rid_by_view) const;
  void allocateTransientResources(TransientPool *pool,
                                  const std::vector<Lifetime> &lifetimes,
                                  std::vector<RID> &rid_by_view);

  std::vector<ResourceEntry> m_entries;
  std::vector<std::unique_ptr<RenderPass>> m_render_passes;
  std::vector<std::unique_ptr<RenderSlot>> m_slots;
  std::vector<PassNode> m_passes;
  std::map<std::string, ResourceView> m_name2view_map;
  std::vector<RID> m_imported_rids;
  std::vector<RID> m_rid_by_view;
};

} // namespace ssme
