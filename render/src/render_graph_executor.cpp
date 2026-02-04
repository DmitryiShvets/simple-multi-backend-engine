#include "render_graph_executor.h"

namespace Render {

RenderGraphExecutor::RenderGraphExecutor(Device* device) : m_rhi_device(device) {}

RenderGraphExecutor::~RenderGraphExecutor() {}

void RenderGraphExecutor::execute(RenderGraph& graph, RID backbuffer) {
    // The logic to execute the graph will go here.
}

} // namespace Render
