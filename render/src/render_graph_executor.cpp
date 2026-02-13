#include "render_graph_executor.h"
#include "command_list.h"
#include "render_graph.h"
#include "render_types.h"

namespace Render {

RenderGraphExecutor::RenderGraphExecutor(Device *device) : m_rhi_device(device) {}

RenderGraphExecutor::~RenderGraphExecutor() {}

void RenderGraphExecutor::execute(RenderGraph &graph, RID back_buffer, RID depth_buffer,
                                  CommandList &cmd, const Rect& render_area) {
  graph.compile();

  const auto &passes = graph.getPasses();
  for (const auto &pass : passes) {
    // In a real implementation, RenderingInfo would be constructed based
    // on the pass's `reads` and `writes` dependencies. For now, we
    // hardcode it to render to the backbuffer.
    RenderingInfo rendering_info{};
    rendering_info.render_area = render_area;
    rendering_info.color_attachments.push_back({
        .texture = back_buffer,
        .load_op = LoadOp::CLEAR,
        .store_op = StoreOp::STORE,
        .clear_value = {0.1f, 0.1f, 0.1f, 1.0f},
        .initial_layout = ImageLayout::UNDEFINED,
        .final_layout = ImageLayout::PRESENT_SRC
    });
    rendering_info.depth_attachment = {
        .texture = depth_buffer,
        .load_op = LoadOp::CLEAR,
        .store_op = StoreOp::DONT_CARE,
        .clear_value = 1.0f,
    };
    cmd.beginRendering(rendering_info);

    auto &callback = pass->getExecuteCallback();
    if (callback) {
      callback(cmd);
    }

    cmd.endRendering();
  }
}

} // namespace Render
