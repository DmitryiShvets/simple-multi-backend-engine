#include "render_graph.h"
#include "core/render_types.h"
#include "core/resource_types.h"
#include "graph/render_gtaph_utils.h"
#include "graph/transient_pool.h"
#include "render_pass.h"

#include <numeric>
#include <queue>
#include <unordered_set>
#include <utility>

namespace ssme {
static ImageLayout ToImageLayout(ResourceState s) {
  switch (s) {
  case ResourceState::COLOR_ATTACHMENT:
    return ImageLayout::COLOR_ATTACHMENT;
  case ResourceState::DEPTH_ATTACHMENT:
    return ImageLayout::DEPTH_ATTACHMENT;
  case ResourceState::SHADER_READ:
    return ImageLayout::SHADER_READ_ONLY;
  case ResourceState::UNORDERED_ACCESS:
    return ImageLayout::UNORDERED_ACCESS;
  case ResourceState::PRESENT:
    return ImageLayout::PRESENT_SRC;
  case ResourceState::TRANSFER_DST:
    return ImageLayout::TRANSFER_DST;
  case ResourceState::TRANSFER_SRC:
    return ImageLayout::TRANSFER_SRC;
  default:
    return ImageLayout::UNDEFINED; // TRANSFER_SRC не имеет аналога в
                                   // ImageLayout
  }
}
static Format ToFormat(ResourceFormat fmt) {
  switch (fmt) {
  case ResourceFormat::RGBA8_UNORM:
    return Format::R8G8B8A8_UNORM;
  case ResourceFormat::RGBA8_SRGB:
    return Format::R8G8B8A8_SRGB;
  case ResourceFormat::RGBA16F:
    return Format::R32G32B32A32_SFLOAT;
  case ResourceFormat::D32F:
    return Format::R32_SFLOAT;
  case ResourceFormat::R8:
    debug_assert(false, "R8 transient not supported yet");
    return Format::R32_SFLOAT;
  }
  return Format::R8G8B8A8_UNORM;
}

static ImageUsage UsageFor(ResourceFormat fmt) {
  if (fmt == ResourceFormat::D32F)
    return ImageUsage::DEPTH_STENCIL;
  constexpr uint32_t RT = static_cast<uint32_t>(ImageUsage::COLOR_ATTACHMENT) |
                          static_cast<uint32_t>(ImageUsage::SHADER_READ) |
                          static_cast<uint32_t>(ImageUsage::TRANSFER_SRC);
  return static_cast<ImageUsage>(RT);
}

// --- RenderGraph Implementation ---

RenderGraph::RenderGraph() {}

RenderGraph::~RenderGraph() {}

ResourceView RenderGraph::addResource(const std::string &name,
                                      const ResourceDesc &desc) {
  if (m_name2view_map.contains(name)) {
    const ResourceView existing = m_name2view_map[name];
    debug_assert(m_entries[existing.index].desc.width == desc.width &&
                     m_entries[existing.index].desc.height == desc.height &&
                     m_entries[existing.index].desc.format == desc.format,
                 "addResource: desc mismatch for '" + name + "'");
    return existing;
  }
  const ResourceEntry entry{
      .desc = desc,
      .versions = {ResourceVersion{}},
      .imported = false,
      .current_state = ResourceState::UNDEFINED,
  };
  m_entries.push_back(entry);
  const ResourceView view{
      .index = static_cast<ResourceIndex>(m_entries.size() - 1),
  };
  m_name2view_map[name] = view;
  return view;
}
ResourceView RenderGraph::getResource(const std::string &name) const {
  return m_name2view_map.at(name);
}
ResourceView RenderGraph::importResource(const std::string &name,
                                         const ResourceDesc &desc,
                                         ResourceState initial_state) {
  if (m_name2view_map.contains(name)) {
    const ResourceView existing = m_name2view_map[name];
    debug_assert(m_entries[existing.index].desc.width == desc.width &&
                     m_entries[existing.index].desc.height == desc.height &&
                     m_entries[existing.index].desc.format == desc.format &&
                     m_entries[existing.index].imported && // только импорт
                     m_entries[existing.index].current_state == initial_state,
                 "importResource: mismatch for '" + name + "'");
    return existing;
  }
  const ResourceEntry entry{
      .desc = desc,
      .versions = {ResourceVersion{}},
      .imported = true,
      .current_state = initial_state,
  };
  m_entries.push_back(entry);
  const ResourceView view{
      .index = static_cast<ResourceIndex>(m_entries.size() - 1),
  };
  m_name2view_map[name] = view;
  return view;
}

// Read: look up who last wrote this resource → add a dependency edge from that
// writer to this pass.
void RenderGraph::read(PassIndex pass, ResourceView view) {
  auto &ver = m_entries[view.index].versions.back(); // current version
  if (ver.hasWriter()) {
    m_passes[pass].depends_on.push_back(ver.writer_pass); // RAW edge
  }
  ver.reader_passes.push_back(pass);    // track who reads this version
  m_passes[pass].reads.push_back(view); // record for barrier insertion
}

// Write: add WAW edge from prev writer + WAR edges from readers, then bump the
// version.
void RenderGraph::write(PassIndex pass, ResourceView view) {
  auto &ver =
      m_entries[view.index].versions.back(); // current version (pre-bump)
  if (ver.hasWriter())
    m_passes[pass].depends_on.push_back(
        ver.writer_pass); // WAW edge: prev writer must finish
  for (PassIndex reader : ver.reader_passes)
    m_passes[pass].depends_on.push_back(
        reader); // WAR edge: reader must finish first
  m_entries[view.index].versions.push_back({}); // bump version
  m_entries[view.index].versions.back().writer_pass =
      pass;                              // this pass owns the new version
  m_passes[pass].writes.push_back(view); // record for barrier insertion
}

// ReadWrite (UAV): depend on previous writer + WAR edges from readers, then
// bump version.
void RenderGraph::readWrite(PassIndex pass, ResourceView view) {
  auto &ver = m_entries[view.index].versions.back();
  if (ver.hasWriter()) {
    m_passes[pass].depends_on.push_back(ver.writer_pass); // RAW edge
  }
  for (PassIndex reader : ver.reader_passes)
    m_passes[pass].depends_on.push_back(reader); // WAR edge
  m_entries[view.index].versions.push_back({});  // bump version (it's a write)
  m_entries[view.index].versions.back().writer_pass = pass;
  m_passes[pass].reads.push_back(
      view); // appears in both lists (for barriers + lifetimes)
  m_passes[pass].writes.push_back(view);
  m_passes[pass].read_writes.push_back(
      view); // marks this handle as UAV for StateForUsage
}

void RenderGraph::read(PassIndex pass, ResourceView view, ResourceState state) {
  m_passes[pass].custom_read_state[view.index] = state;
  read(pass, view);
}
void RenderGraph::write(PassIndex pass, ResourceView view,
                        ResourceState state) {
  m_passes[pass].custom_write_state[view.index] = state;
  write(pass, view);
}

// Deduplicate raw dependsOn edges and build forward adjacency list (successors)
// for Kahn's algorithm.
void RenderGraph::buildEdges() {
  for (PassIndex i = 0; i < m_passes.size(); i++) {
    std::unordered_set<PassIndex> seen;
    for (PassIndex dep : m_passes[i].depends_on) {
      if (seen.insert(dep).second) // first time seeing this edge?
      {
        m_passes[dep].successors.push_back(i); // forward link: dep → i
        m_passes[i].in_degree++;               // i has one more incoming edge
      }
    }
  }
}

// Kahn's algorithm: dequeue zero-in-degree passes → valid execution order
// respecting all dependencies.
std::vector<PassIndex> RenderGraph::topoSort() {
  // A → B если:
  //   A.writes(R) и B.reads(R)   (RAW)
  //   A.writes(R) и B.writes(R)  (WAW)
  //   A.reads(R)  и B.writes(R)  (WAR)
  std::queue<PassIndex> q;
  std::vector<uint32_t> inDeg(m_passes.size());
  for (PassIndex i = 0; i < m_passes.size(); i++) {
    inDeg[i] = m_passes[i].in_degree;
    if (inDeg[i] == 0)
      q.push(i); // no dependencies → ready immediately
  }
  std::vector<PassIndex> order;
  while (!q.empty()) {
    PassIndex cur = q.front();
    q.pop();
    order.push_back(cur);
    for (PassIndex succ : m_passes[cur].successors) {
      if (--inDeg[succ] == 0) // all of succ's dependencies done?
        q.push(succ);         // succ is now ready
    }
  }
  // If we didn't visit every pass, the graph has a cycle, invalid.
  assert(order.size() == m_passes.size() && "Cycle detected in render graph!");
  return order;
}

// Walk sorted passes, compare tracked state to each resource's needed state,
// record transitions.
std::vector<std::vector<ResourceBarrier>>
RenderGraph::computeBarriers(const std::vector<PassIndex> &sorted,
                             const std::vector<BlockIndex> &mapping) {
  std::vector<std::vector<ResourceBarrier>> result(sorted.size());

  // blockOwner[block] = which virtual resource currently occupies it.
  std::vector<ResourceIndex> blockOwner;
  {
    BlockIndex maxBlock = 0;
    for (auto m : mapping)
      if (m != UINT32_MAX)
        maxBlock = std::max(maxBlock, m + 1);
    blockOwner.assign(maxBlock, UINT32_MAX);
  }

  for (PassIndex orderIdx = 0; orderIdx < sorted.size(); orderIdx++) {
    PassIndex passIdx = sorted[orderIdx];
    if (!m_passes[passIdx].alive)
      continue;

    // --- Collect unique handles (ReadWrite puts h in both reads & writes) ---
    std::vector<std::pair<ResourceView, bool>> unique; // {handle, isWrite}
    std::unordered_set<ResourceIndex> seen;
    for (auto &h : m_passes[passIdx].reads)
      if (seen.insert(h.index).second)
        unique.push_back({h, false});
    for (auto &h : m_passes[passIdx].writes) {
      if (seen.insert(h.index).second) {
        unique.push_back({h, true});
      } else {
        // already in reads — upgrade to write (UAV)
        for (auto &[uh, w] : unique)
          if (uh.index == h.index) {
            w = true;
            break;
          }
      }
    }

    // --- Phase 1: aliasing barriers (block changes occupant) ---
    for (auto &[h, _] : unique) {
      BlockIndex block = mapping[h.index];
      if (block == UINT32_MAX)
        continue;
      if (blockOwner[block] != UINT32_MAX && blockOwner[block] != h.index) {
        result[orderIdx].push_back({h.index, ResourceState::UNDEFINED,
                                    ResourceState::UNDEFINED, true,
                                    blockOwner[block]});
      }
      blockOwner[block] = h.index;
    }

    // --- Phase 2: state-transition barriers ---
    for (auto &[h, isWrite] : unique) {
      ResourceState needed = stateForUsage(passIdx, h, isWrite);
      if (m_entries[h.index].current_state != needed) {
        result[orderIdx].push_back(
            {h.index, m_entries[h.index].current_state, needed});
        m_entries[h.index].current_state = needed;
      }
    }
  }
  // for (uint32_t o = 0; o < sorted.size(); o++) {
  //   PassIndex p = sorted[o];
  //   printf("  [pass %s idx %u]:", m_render_passes[p]->name().data(), p);
  //   for (auto &b : result[o])
  //     printf(" r%u(%d->%d)%s", b.resource_index,
  //            (int)b.old_state, (int)b.new_state, b.is_aliasing ? "[alias]" : "");
  //   printf("\n");
  // }


  uint32_t total = 0;
  for (auto &v : result)
    total += static_cast<uint32_t>(v.size());
  // printf("  Barriers computed: %u transition(s) across %u passes\n", total,
  //        static_cast<uint32_t>(sorted.size()));
  return result;
}

// Replay precomputed transitions, in production this calls the GPU API.
void RenderGraph::emitBarriers(CommandList &cmd,
                               const std::vector<ResourceBarrier> &barriers,
                               const std::vector<RID> &rid_by_view) {
  BarrierInfo info;
  for (const auto &b : barriers) {
    if (b.is_aliasing)
      continue; // RHI BarrierInfo пока не умеет aliasing-барьеры
    if (b.old_state == b.new_state) continue; // НОВОЕ: no-op переход . это баг???
    info.image_barriers.push_back({.image = rid_by_view[b.resource_index],
                                   .old_layout = ToImageLayout(b.old_state),
                                   .new_layout = ToImageLayout(b.new_state)});
  }
  if (!info.image_barriers.empty())
    cmd.pipelineBarrier(info);
}

// In declared passes + virtual resources + read/write edges
// Out ordered passes · aliased memory · barrier list · physical bindings

// Full compile pipeline: sort → cull → precompute barriers. Returns a
// self-contained plan.
CompiledPlan RenderGraph::compile(TransientPool *pool) {
  // 1. подготавливаем граф для топологической сортировки
  buildEdges();
  // 2. Топологическая сортировка Sort passes into dependency order
  auto sorted = topoSort();
  // 3. Dead code elimination. Cull passes whose outputs are never read
  cull(sorted);
  // 4. Lifetime computation. Scan lifetimes: record each transient resource's
  // first and last use
  auto lifetimes = scanLifetimes(sorted); // when is each resource alive?
  // 5. Alias: assign non-overlapping resources to shared memory slots
  auto mapping =
      aliasResources(lifetimes); // share memory where lifetimes don't overlap
  // 6. Compute barriers: insert transitions at every resource state change
  auto barriers = computeBarriers(
      sorted, mapping); // extended: also emits aliasing transitions
  m_rid_by_view.assign(m_entries.size(), RID::INVALID);

  for (ResourceIndex i = 0; i < m_entries.size(); i++) {
    if (m_entries[i].imported) {
      if (i < m_imported_rids.size())
        m_rid_by_view[i] = m_imported_rids[i];
      debug_assert(m_rid_by_view[i].isValid(), "import without bound RID");
    }
  }
  // 7. Resource allocation (transient)
  allocateTransientResources(pool, lifetimes, m_rid_by_view);
  const CompiledPlan plan{std::move(sorted), std::move(mapping),
                          std::move(barriers)};
  buildCompiledPasses(plan, m_rid_by_view);

  return plan;
}

// Dead-code elimination: walk backward from the final output pass, marking
// dependencies alive.
void RenderGraph::cull(const std::vector<PassIndex> &sorted) {
  if (sorted.empty())
    return;
  // Ищем финальные pass'ы — те что пишут в backbuffer/swapchain
  for (int i = 0; i < m_render_passes.size(); i++) {
    if (m_render_passes[i]->name() == "Present") {
      m_passes[i].alive = true;
      break;
    }
  }
  // Present pass = the final output (e.g. Present)
  for (int i = static_cast<int>(sorted.size()) - 1; i >= 0; i--) {
    if (!m_passes[sorted[i]].alive)
      continue; // skip dead passes
    for (PassIndex dep : m_passes[sorted[i]].depends_on)
      m_passes[dep].alive = true; // my dependency is needed → keep it alive
  }
}

// Record each resource's first/last use in sorted order, non-overlapping
// intervals can share memory.
std::vector<Lifetime>
RenderGraph::scanLifetimes(const std::vector<PassIndex> &sorted) {
  std::vector<Lifetime> life(m_entries.size());
  // Imported resources (e.g. swapchain) are externally owned, exclude from
  // aliasing.
  for (ResourceIndex i = 0; i < m_entries.size(); i++) {
    if (m_entries[i].imported)
      life[i].is_transient = false;
  }
  // Update first/last use for every resource each surviving pass touches.
  for (PassIndex order = 0; order < sorted.size(); order++) {
    PassIndex passIdx = sorted[order];
    if (!m_passes[passIdx].alive)
      continue;
    for (auto &h : m_passes[passIdx].reads) {
      life[h.index].first_use = std::min(life[h.index].first_use, order);
      life[h.index].last_use = std::max(life[h.index].last_use, order);
    }
    for (auto &h : m_passes[passIdx].writes) {
      life[h.index].first_use = std::min(life[h.index].first_use, order);
      life[h.index].last_use = std::max(life[h.index].last_use, order);
    }
  }
  return life;
}

// Greedy first-fit: sort by firstUse, reuse any free block that fits, else
// allocate a new one.
std::vector<BlockIndex>
RenderGraph::aliasResources(const std::vector<Lifetime> &lifetimes) {
  std::vector<PhysicalBlock> freeList;
  std::vector<BlockIndex> mapping(m_entries.size(), UINT32_MAX);
  // Process resources in the order they're first used.
  std::vector<ResourceIndex> indices(m_entries.size());
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(),
            [&](ResourceIndex a, ResourceIndex b) {
              return lifetimes[a].first_use < lifetimes[b].first_use;
            });
  for (ResourceIndex resIdx : indices) {
    if (!lifetimes[resIdx].is_transient)
      continue; // skip imported resources
    if (lifetimes[resIdx].first_use == UINT32_MAX)
      continue; // never used
    uint32_t needed = AllocSize(m_entries[resIdx].desc);
    bool reused = false;
    // Scan existing blocks, can we reuse one that's now free?
    for (BlockIndex b = 0; b < freeList.size(); b++) {
      if (freeList[b].avail_after < lifetimes[resIdx].first_use &&
          freeList[b].size_bytes >= needed) {
        mapping[resIdx] = b; // reuse this block
        freeList[b].avail_after =
            lifetimes[resIdx].last_use; // extend occupancy
        reused = true;
        break;
      }
    }
    if (!reused) { // no fit found → allocate a new physical block
      mapping[resIdx] = static_cast<BlockIndex>(freeList.size());
      freeList.push_back({needed, lifetimes[resIdx].last_use});
    }
  }
  return mapping;
}

// Infer the ResourceState a pass needs for a given resource handle.
ResourceState RenderGraph::stateForUsage(PassIndex passIdx, ResourceView view,
                                         bool isWrite) const {
  const auto &node = m_passes[passIdx];
  // check if node hase overrides
  if (isWrite) {
    auto it = node.custom_write_state.find(view.index);
    if (it != node.custom_write_state.end())
      return it->second;
  } else {
    auto it = node.custom_read_state.find(view.index);
    if (it != node.custom_read_state.end())
      return it->second;
  }
  // default
  for (auto &rw : node.read_writes)
    if (rw.index == view.index)
      return ResourceState::UNORDERED_ACCESS;
  if (isWrite)
    return (m_entries[view.index].desc.format == ResourceFormat::D32F)
               ? ResourceState::DEPTH_ATTACHMENT
               : ResourceState::COLOR_ATTACHMENT;
  return ResourceState::SHADER_READ;
}

// render_graph.cpp
void RenderGraph::bindImport(ResourceView view, RID rid) {
  if (view.index >= m_imported_rids.size())
    m_imported_rids.resize(view.index + 1, RID::INVALID);
  m_imported_rids[view.index] = rid;
}

// Pure playback, emit precomputed barriers, call execute lambdas. No analysis.
void RenderGraph::execute(CommandList &cmd, const CompiledPlan &plan,
                          const Rect &render_area) {
  // FOR EACH PASS
  //      submit precomputed barriers
  //      begin render pass
  //      call execute() lambda: draw calls, dispatches, copies
  //      end render pass

  for (PassIndex orderIdx = 0; orderIdx < plan.sorted.size(); orderIdx++) {
    PassIndex passIdx = plan.sorted[orderIdx];
    if (!m_passes[passIdx].alive)
      continue;
    emitBarriers(cmd, plan.barriers[orderIdx], m_rid_by_view);
    const RenderingInfo &info0 = m_render_passes[passIdx]->rendering_info;
    const bool has_attachments =
        !info0.color_attachments.empty() ||
        info0.depth_attachment.texture.isValid();
    if (has_attachments) {
      RenderingInfo info = info0;
      info.render_area = render_area;
      cmd.beginRendering(info);
      cmd.setViewport({.x = 0.0f, .y = 0.0f,
                       .width = (float)render_area.width,
                       .height = (float)render_area.height,
                       .minDepth = 0.0f, .maxDepth = 1.0f});
      cmd.setScissor(render_area);

    }
    auto &cb = m_render_passes[passIdx]->getExecuteCallback();
    if (cb)
      cb(cmd);
    if (has_attachments)
      cmd.endRendering();
  }
}
void RenderGraph::reset() {
  m_imported_rids.clear();
  m_passes.clear();
  m_render_passes.clear();
  m_slots.clear();
  m_entries.clear();
  m_name2view_map.clear();
}

void RenderGraph::build() {
  for (auto &slot : m_slots) {
    slot->onSetup(*this);
  }
  for (auto &slot : m_slots) {
    slot->onResolve(*this);
  }
  for (auto &slot : m_slots) {
    slot->onBuild(*this);
  }
}

RenderPass &RenderGraph::addPass(const std::string &name) {
  PassNode node;
  node.index = static_cast<PassIndex>(m_passes.size());
  m_passes.push_back(std::move(node));
  m_render_passes.push_back(RenderPass::Create(name, node.index, this));
  return *m_render_passes.back();
}

void RenderGraph::buildCompiledPasses(const CompiledPlan &plan,
                                      const std::vector<RID> &rid_by_view) {
  for (PassIndex orderIdx = 0; orderIdx < plan.sorted.size(); orderIdx++) {
    PassIndex passIdx = plan.sorted[orderIdx];
    RenderPass &pass = *m_render_passes[passIdx];
    PassNode &pass_node = m_passes[passIdx];
    if (!pass_node.alive)
      continue; // cull уже выкинул мёртвых
    pass.rendering_info = buildRenderingInfo(pass_node, rid_by_view);
  }
}

bool RenderGraph::isFirstWriter(PassIndex passIdx, ResourceView view) const {
  const auto &versions = m_entries[view.index].versions;
  // versions[0] — сид, первый реальный write — versions[1]
  return versions.size() < 2 || versions[1].writer_pass == passIdx;
}

RenderingInfo
RenderGraph::buildRenderingInfo(const PassNode &pass,
                                const std::vector<RID> &rid_by_view) const {
  RenderingInfo info;
  for (const ResourceView &v : pass.writes) {
    auto itw = pass.custom_write_state.find(v.index);
    if (itw != pass.custom_write_state.end() &&
        itw->second != ResourceState::COLOR_ATTACHMENT &&
        itw->second != ResourceState::DEPTH_ATTACHMENT)
      continue; // copy/transfer — это не аттачмент
    bool is_uav = false;
    for (const ResourceView &rw : pass.read_writes)
      if (rw.index == v.index) {
        is_uav = true;
        break;
      }
    if (is_uav)
      continue; // storage image, не аттачмент
    const bool first_writer = isFirstWriter(pass.index, v);
    const RenderPass &rp = *m_render_passes[pass.index];
    LoadOp load = rp.getLoadOp();
    if (load == LoadOp::AUTO) {
      load = first_writer ? LoadOp::CLEAR : LoadOp::LOAD;
    }
    const ResourceEntry &entry = m_entries[v.index];
    // TODO: Придумать по лучше решение
    const RID rid = rid_by_view[v.index];
    if (entry.desc.format == ResourceFormat::D32F) {
      info.depth_attachment = {
          .texture = rid,
          .load_op = load,
          .store_op = StoreOp::STORE,
          .clear_value = 1.0f,
      };
    } else {
      info.color_attachments.push_back({
          .texture = rid,
          .load_op = load,
          .store_op = StoreOp::STORE,
          /* из ResourceDesc или дефолт */
          .clear_value = {0.1f, 0.1f, 0.1f, 1.0f},
          .initial_layout = load == LoadOp::CLEAR
                                ? ImageLayout::UNDEFINED
                                : ImageLayout::COLOR_ATTACHMENT,
          .final_layout = ImageLayout::COLOR_ATTACHMENT,
      });
    }
  }
  return info;
}

void RenderGraph::allocateTransientResources(
    TransientPool *pool, const std::vector<Lifetime> &lifetimes,
    std::vector<RID> &rid_by_view) {
  for (const auto &[name, view] : m_name2view_map) {
    const ResourceIndex i = view.index;
    if (m_entries[i].imported) // свапчейн — уже забинден
      continue;
    if (lifetimes[i].first_use == UINT32_MAX) // отцеллен/не используется
      continue;
    const auto &desc = m_entries[i].desc;
    TextureDesc tex_desc{};
    tex_desc.width = desc.width;
    tex_desc.height = desc.height;
    tex_desc.format = ToFormat(desc.format);
    tex_desc.usage = UsageFor(desc.format);
    tex_desc.generate_mips = false;
    rid_by_view[i] = pool->getOrCreate(name, tex_desc);
  }
}

} // namespace ssme
