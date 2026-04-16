#include "pipeline.h"
#include "core/resource_types.h"
#include "render_device.h"
#include "resource.h"
#include "utils/debug_assert.h"
#include <string>

namespace ssme {

Pipeline::Pipeline(std::string id, const VecRefRD &devices,
                   const PipelineParams &data)
    : Resource(id, devices), m_params(data) {
  m_vert_shader = std::move(m_params.vert_shader);
  m_frag_shader = std::move(m_params.frag_Shader);
}
Pipeline::~Pipeline() {}

uint32_t Pipeline::doPrepare() {
  uint32_t requred_components = 2;
  // fill pipeline layout
  auto append = [&](const auto &source, auto &dest) {
    dest.insert(dest.end(), source.begin(), source.end());
  };
  append(m_vert_shader.get()->getDescriptorLayouts(),
         m_pl_layout_desc.descriptor_layouts);
  append(m_frag_shader.get()->getDescriptorLayouts(),
         m_pl_layout_desc.descriptor_layouts);

  append(m_vert_shader.get()->getPushConstants(),
         m_pl_layout_desc.push_constant_ranges);
  append(m_frag_shader.get()->getPushConstants(),
         m_pl_layout_desc.push_constant_ranges);

  // 1. First create/find PipelineLayout
  // add hash calculation for m_pl_layout_desc.
  // if already exists return 1, if not return 2
  // then fully assemble m_pl_desc and check its hash
  // if exists return 0, if not return 2.
  // if hash found for m_pl_layout_desc assign m_pl_layout_id from hash map
  // if hash found for m_pl_desc assign m_pl_id from hash map
  m_pl_desc.vertex_layout = m_params.vertex_layout;
  m_pl_desc.vert_shader_module = m_vert_shader.get()->getModuleId();
  m_pl_desc.frag_shader_module = m_frag_shader.get()->getModuleId();
  // check if the same layout already exists;
  std::size_t layout_hash = m_pl_layout_desc.hash();
  RID hashed_layout = m_devices.at(0).get().containsPipelineLayout(layout_hash);
  if (hashed_layout) {
    m_pl_layout_id = hashed_layout;
    m_pl_desc.pl_layout_id = m_pl_layout_id;
    m_requred_components--;
    m_need_to_create_pl_lyout = false;
  }

  if (m_pl_layout_id) {
    std::size_t pipeline_hash = m_pl_desc.hash();
    RID hashed_pipeline =
        m_devices.at(0).get().containsGraphicsPipeline(pipeline_hash);
    if (hashed_pipeline) {
      m_pl_id = hashed_layout;
      m_requred_components--;
      m_need_to_create_pl = false;
    }
  }
  // if layout and pipeline already exists than resource is loaded;
  m_loaded = !(m_need_to_create_pl || m_need_to_create_pl_lyout);

  // how much RIDs resource manager must allocate
  return m_requred_components;
}

void Pipeline::doSetup(const VecRID &rids) {
  debug_assert(rids.size() == m_requred_components,
               "Mesh requires exactly " + std::to_string(m_requred_components) +
                   " RIDs (VB and IB)");
  if (rids.size() == 2) {
    m_pl_id = rids[0];
    m_pl_layout_id = rids[1];
  } else if (rids.size() == 1) {
    m_pl_id = rids[0];
    debug_assert(m_pl_layout_id.isValid(),
                 "Invalide RID of pipeline layout. It must be logical error!");
  }
}

bool Pipeline::doLoad() {
    auto requirements = m_vert_shader.get()->getVertexInputRequirements();

    // TODO: do this during rendering Check compatibility with mesh layout
    // if (!m_params.vertex_layout.isCompatibleWith(requirements)) {
    //   debug_assert(false, "Pipeline validation failed: mesh and shader are incompatible");
    //   return false;
    // }

  for (auto &rd : m_devices) {
    // create pipeline layout
    if (m_need_to_create_pl_lyout) {
      rd.get().createPipelineLayout(m_pl_layout_desc, m_pl_layout_id);
      m_pl_desc.pl_layout_id = m_pl_layout_id;
    }

    // fill pipeline create info
    if (m_need_to_create_pl) {
      debug_assert(
          m_pl_layout_id.isValid(),
          "Invalide RID of pipeline layout. It must be logical error!");
      rd.get().createGraphicsPipeline(m_pl_desc, m_pl_id);
    }
  }
  return true;
}

bool Pipeline::doUnload() {
  for (auto &rd : m_devices) {
    rd.get().destroyBuffer(m_pl_id);
    rd.get().destroyBuffer(m_pl_layout_id);
  }
  m_pl_id = RID::INVALID;
  m_pl_layout_id = RID::INVALID;
  return true;
}
} // namespace ssme
