#pragma once

#include "core/resource_types.h"
#include "resource.h"
#include <cstdint>
#include <string>

namespace ssme {

class Buffer : public Resource {
public:
  static constexpr ResourceId ID = ResourceId::MESH;
  static constexpr uint32_t COMPONENTS = 1;

  explicit Buffer(std::string id, const VecRefRD &devices,
                  const BufferDesc &desc);
  ~Buffer() override;

  void doSetup(const VecRID& rids) override;
  bool doLoad() override;
  bool doUnload() override;

private:
  BufferDesc m_buffer_desc;
  RID m_buffer_id;
};
} // namespace ssme
