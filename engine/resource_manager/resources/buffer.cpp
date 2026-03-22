#include "buffer.h"
#include "render_device.h"
#include "utils/debug_assert.h"
#include <imgui/imgui.h>

namespace ssme {

Buffer::Buffer(std::string id, const VecRefRD &devices, const BufferDesc &desc)
    : Resource(id, devices) {}

Buffer::~Buffer() {}

void Buffer::doSetup(const VecRID &rids) {
  debug_assert(rids.size() == COMPONENTS,
               "The number of RIDs and the number of components must match");
  m_buffer_id = rids[0];
}

bool Buffer::doLoad() {
  for (auto &rd : m_devices) {
    rd.get().createBuffer(m_buffer_desc, m_buffer_id);
  }
  return true;
}
bool Buffer::doUnload() {
  for (auto &rd : m_devices) {
    rd.get().destroyBuffer(m_buffer_id);
  }
  m_buffer_id = RID::INVALID;
  return true;
}
} // namespace ssme
