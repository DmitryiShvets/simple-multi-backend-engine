#pragma once

#include "core/resource_types.h"
#include "core/rid.h"
#include "core/vertex_layout.h"
#include "resource.h"
#include <glm/glm.hpp>

/**
 * @brief Mesh resource
 *
 * Goals and requirements:
 * - Format universality: Class doesn't depend on specific vertex structure
 *   (VertexP, VertexPN, etc.). It stores its own VertexLayout.
 * - GPU resource encapsulation: All RIDs (vertex buffer, index buffer) are
 *   created and stored internally. External world (ECS) sees only high-level object.
 * - Indexing support: Mandatory use of Index Buffer for rendering optimization.
 * - Geometric metadata: Stores AABB (Bounding Box) for frustum culling and physics.
 * - ResourceManager integration: Inherits from Resource for automatic lifetime
 *   management (GPU deletion when ECS reference count reaches zero).
 * - Backend-agnostic: doLoad creates buffers for all registered backends
 *   (Vulkan/OpenGL) via RenderDevice.
 */

namespace ssme {

class Mesh : public Resource {
public:
  using ParamsType = MeshDesc;
  static constexpr ResourceId ID = ResourceId::MESH;
  static constexpr uint32_t COMPONENTS = 2; // VB + IB

  Mesh(std::string id, const VecRefRD &devices, const MeshDesc &desc);
  ~Mesh() override;

  // Resource interface implementation
  uint32_t doPrepare() override;
  void doSetup(const VecRID &rids) override;
  bool doLoad() override;
  bool doUnload() override;

  // Getters for renderer
  RID getVertexBuffer() const { return m_vb_id; }
  RID getIndexBuffer() const { return m_ib_id; }
  uint32_t getIndexCount() const { return m_index_count; }
  const VertexLayout &getVertexLayout() const { return m_layout; }
  const AABB &getBounds() const { return m_bounds; }

private:
  MeshDesc m_desc;
  RID m_vb_id = RID::INVALID;
  RID m_ib_id = RID::INVALID;
  uint32_t m_index_count = 0;
  VertexLayout m_layout;
  AABB m_bounds;
};

} // namespace ssme
