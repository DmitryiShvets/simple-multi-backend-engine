#pragma once

#include "core/resource_types.h"
#include "core/rid.h"
#include "resource.h"

namespace ssme {
class Buffer : public Resource {
public:
  static constexpr ResourceId ID = ResourceId::MESH;
  explicit Buffer(const std::string &id,
                  const std::vector<RenderDevice *> &devices);
  explicit Buffer(const BufferDesc &desc,
                  const std::vector<RenderDevice *> &devices);
  ~Buffer() override {
    unload(); // Ensure proper cleanup when object is destroyed
  }

  bool doLoad() override;
  bool doUnload() override;

private:
  BufferDesc m_buffer_desc;
  RID m_buffer_id;
};
} // namespace ssme
