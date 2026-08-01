#include "graph/render_pass.h"
#include "graph/render_graph.h"
#include <utility>

namespace ssme {

// --- RenderPass Factory ---
std::unique_ptr<RenderPass> RenderPass::Create(const std::string &name,
                                               uint32_t index,
                                               RenderGraph *graph) {
  // Use 'new' to access the private constructor
  return std::unique_ptr<RenderPass>(new RenderPass(name, index, graph));
}

// --- RenderPass Implementation ---

RenderPass::RenderPass(const std::string &name, uint32_t index,
                       RenderGraph *graph)
    : m_name(name), m_graph(graph), m_index(index) {}

RenderPass &RenderPass::read(ResourceView resource) {
  m_graph->read(m_index, resource);
  return *this;
}
RenderPass &RenderPass::write(ResourceView resource) {
  m_graph->write(m_index, resource);
  return *this;
}
RenderPass &RenderPass::readWrite(ResourceView resource) {
  m_graph->readWrite(m_index, resource);
  return *this;
}
RenderPass &RenderPass::setLoadOp(LoadOp op) {
  m_load_op = op;
  return *this;
}

void RenderPass::setExecuteCallback(
    std::function<void(CommandList &cmd)> &&callback) {
  m_execute_callback = std::move(callback);
}
const ExecuteCallback &RenderPass::getExecuteCallback() const {
  return m_execute_callback;
}
const std::string &RenderPass::name() const { return m_name; }
} // namespace ssme
