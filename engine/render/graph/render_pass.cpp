#include "render_pass.h"
#include <utility>

namespace ssme {

// --- RenderPass Factory ---
std::unique_ptr<RenderPass> RenderPass::Create(const std::string &name,
                                               RenderGraph *graph) {
  // Use 'new' to access the private constructor
  return std::unique_ptr<RenderPass>(new RenderPass(name, graph));
}

// --- RenderPass Implementation ---

RenderPass::RenderPass(const std::string &name, RenderGraph *graph)
    : m_name(name), m_graph(graph) {}

RenderPass &RenderPass::reads(RID resource) {
  m_reads.push_back(resource);
  return *this;
}

RenderPass &RenderPass::writes(RID resource) {
  m_writes.push_back(resource);
  return *this;
}

void RenderPass::setExecuteCallback(
    std::function<void(CommandList &cmd)> &&callback) {
  m_execute_callback = std::move(callback);
}

} // namespace ssme
