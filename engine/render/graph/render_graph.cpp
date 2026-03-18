#include "render_graph.h"
#include "render_pass.h"
#include <utility>

namespace ssme {

// --- RenderGraph Implementation ---

RenderGraph::RenderGraph() {}

RenderGraph::~RenderGraph() {}

RenderPass &RenderGraph::addPass(const std::string &name) {
  m_passes.push_back(RenderPass::Create(name, this));
  return *m_passes.back();
}

void RenderGraph::compile() {
  // In a real implementation, this would:
  // 1. Validate the graph (e.g., check for cycles).
  // 2. Perform topological sort on the passes.
  // 3. Resolve resource dependencies and manage lifetimes (transients).
  // 4. Determine necessary barriers between passes.
  // For now, we just mark it as compiled.
  m_is_compiled = true;
}

} // namespace ssme
