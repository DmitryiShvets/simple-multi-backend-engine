#pragma once

#include "resource.h"
#include "core/rid.h"
#include "core/resource_types.h"

namespace ssme {

class Buffer : public Resource {
public:
  static constexpr ResourceId ID = ResourceId::MESH;

  explicit Buffer(const BufferDesc &desc, const VecRefRD &devices);
  ~Buffer() override;

  bool doLoad() override;
  bool doUnload() override;

private:
  BufferDesc m_buffer_desc;
  RID m_buffer_id;
};
} // namespace ssme
