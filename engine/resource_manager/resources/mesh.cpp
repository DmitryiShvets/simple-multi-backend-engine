#include "mesh.h"
#include "render_device.h"
#include "utils/debug_assert.h"

namespace ssme {

Mesh::Mesh(std::string id, const VecRefRD &devices, const MeshDesc &desc)
    : Resource(id, devices), m_desc(desc), m_layout(desc.layout),
      m_bounds(desc.bounds) {
  m_index_count = static_cast<uint32_t>(desc.indices.size());
}

Mesh::~Mesh() {
  // doUnload();
}

uint32_t Mesh::doPrepare() { return COMPONENTS; }

void Mesh::doSetup(const VecRID &rids) {
  debug_assert(rids.size() == COMPONENTS,
               "Mesh requires exactly 2 RIDs (VB and IB)");
  m_vb_id = rids[0];
  m_ib_id = rids[1];
}

bool Mesh::doLoad() {
  if (m_desc.vertices.empty())
    return false;

  for (auto &rd : m_devices) {
    // 1. Create Vertex Buffer via first RID
    rd.get().createBuffer(
        BufferDesc{
            .size = m_desc.vertices.size(),
            .element_size = m_desc.vertices.size() / m_layout.getStride(),
            .usage = static_cast<uint32_t>(BufferUsage::VERTEX_BUFFER),
            .initial_data = m_desc.vertices.data(),
            .layout = m_layout,
        },
        m_vb_id);

    // 2. Create Index Buffer via second RID
    if (!m_desc.indices.empty()) {
      rd.get().createBuffer(
          BufferDesc{
              .size = m_desc.indices.size() * sizeof(uint32_t),
              .element_size = sizeof(uint32_t),
              .usage = static_cast<uint32_t>(BufferUsage::INDEX_BUFFER),
              .initial_data = m_desc.indices.data(),
          },
          m_ib_id);
    }
  }

  // Clear system memory after uploading to GPU (optional)
  m_desc.vertices.clear();
  m_desc.vertices.shrink_to_fit();
  m_desc.indices.clear();
  m_desc.indices.shrink_to_fit();

  return true;
}

bool Mesh::doUnload() {
  for (auto &rd : m_devices) {
    rd.get().destroyBuffer(m_vb_id);
    rd.get().destroyBuffer(m_ib_id);
  }
  m_vb_id = RID::INVALID;
  m_ib_id = RID::INVALID;
  return true;
}

} // namespace ssme
