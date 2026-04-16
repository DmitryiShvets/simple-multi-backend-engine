#include "renderer.h"

#include "command_list.h"
#include "core/render_types.h"
#include "render_item.h"

namespace ssme {

void IRenderer::draw(CommandList &cmd, const RenderItem &item) {
  if (!item.pipeline.isValid())
    return;

  // 1. Pipeline (Shaders + States)
  cmd.setGraphicsPipeline(item.pipeline);

  // 2. Vertex Buffers (can be multiple for Vertex Fetching)
  for (uint32_t i = 0; i < item.vertex_buffers.size(); ++i) {
    if (item.vertex_buffers[i].isValid()) {
      cmd.setVertexBuffer(i, item.vertex_buffers[i], 0);
    }
  }

  // 3. Descriptor Sets (Uniforms, Textures)
  for (uint32_t set_idx = 0; set_idx < item.descriptor_sets.size(); ++set_idx) {
    if (item.descriptor_sets[set_idx].isValid()) {
      cmd.setDescriptorSet(set_idx, item.getDescriptor(set_idx),
                           item.pipeline);
    }
  }

  // 4. Push Constants
  if (!item.push_constants.empty()) {
        auto it = item.push_constants.find("model_mat");
        assert(it != item.push_constants.end() &&
               "DrawingData: model_mat push constant is required");
        cmd.setPushConstant(item.pipeline, item.push_constants.at("model_mat"),
                            static_cast<uint32_t>(ShaderStage::VERTEX));

  }
  // 5. Choose drawing method: Indexed vs Non-Indexed
  if (item.draw_cmd.index_count > 0) {
    if (item.index_buffer.isValid()) {
        // TODO
      cmd.setIndexBuffer(item.index_buffer, 0, IndexType::UINT32);

      cmd.drawIndexed(item.draw_cmd.index_count, item.draw_cmd.instance_count,
                      item.draw_cmd.first_index, item.draw_cmd.vertex_offset,
                      item.draw_cmd.first_instance);
    }
  } else {
    cmd.draw(item.draw_cmd.vertex_count, item.draw_cmd.instance_count,
             item.draw_cmd.first_vertex, item.draw_cmd.first_instance);
  }
}
} // namespace ssme
