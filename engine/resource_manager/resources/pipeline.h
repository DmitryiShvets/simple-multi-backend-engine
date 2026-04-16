#pragma once
#include "core/resource_types.h"
#include "core/vertex_layout.h"
#include "resource.h"
#include "resource_handle.h"
#include "resource_handle_base.h"
#include "resources/shader.h"
#include "utils/hash_utils.h"

#include <cstdint>
#include <format>

namespace ssme {

struct PipelineParams {
  ResourceHandle<ShaderModule> vert_shader;
  ResourceHandle<ShaderModule> frag_Shader;
  VertexLayout vertex_layout;
  PrimitiveTopology primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
  RasterizationStateDesc rasterization_state;
  DepthStencilStateDesc depth_stencil_state;

  std::size_t hash() const {
    std::size_t h = 0;
    // Shader IDs
    hash_combine(h, vert_shader->id(), frag_Shader->id());
    // Vertex layout + pipeline states
    hash_combine(h, static_cast<uint32_t>(primitive_topology),
                 rasterization_state.hash(), depth_stencil_state.hash());
    return h;
  }

  std::string uuid() const { return std::format("pipeline_{:016x}", hash()); }

  bool operator==(const PipelineParams &other) const {
    return vert_shader->id() == other.vert_shader->id() &&
           frag_Shader->id() == other.frag_Shader->id() &&
           primitive_topology == other.primitive_topology &&
           rasterization_state == other.rasterization_state &&
           depth_stencil_state == other.depth_stencil_state;
  }
};

class Pipeline : public Resource {
public:
  using ParamsType = PipelineParams;
  static constexpr ResourceId ID = ResourceId::MESH;
  static constexpr uint32_t COMPONENTS = 2; // layout + pipeline

  Pipeline(std::string id, const VecRefRD &devices, const PipelineParams &data);
  ~Pipeline() override;

  // Resource interface implementation
  uint32_t doPrepare() override;
  void doSetup(const VecRID &rids) override;
  bool doLoad() override;
  bool doUnload() override;

  RID getPipelineID() const { return m_pl_id; }

private:
  PipelineParams m_params;

  ResourceHandle<ShaderModule> m_vert_shader;
  ResourceHandle<ShaderModule> m_frag_shader;

  GraphicsPipelineDesc m_pl_desc;      // out
  PipelineLayoutDesc m_pl_layout_desc; // out

  RID m_pl_id;
  RID m_pl_layout_id;

  uint32_t m_requred_components = COMPONENTS;

  bool m_need_to_create_pl = true;
  bool m_need_to_create_pl_lyout = true;
};

} // namespace ssme
