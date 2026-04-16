#include "uniform_block.h"
#include "core/resource_types.h"
#include "render_device.h"
#include "utils/debug_assert.h"
#include <utility>

namespace ssme {

UniformBuffer::UniformBuffer(std::string id, const VecRefRD &devices,
                           const UniformBlockDesc &desc)
    : Resource(id, devices), m_label(std::move(desc.name)),
      m_buffer_size(desc.size), m_packed_data(std::move(desc.data)),
      m_layout(std::move(desc.layout)) {}
UniformBuffer::~UniformBuffer() {}

uint32_t UniformBuffer::doPrepare() { return COMPONENTS; }

void UniformBuffer::doSetup(const VecRID &rids) {
  debug_assert(rids.size() == COMPONENTS,
               "Uniform block requires exactly 1 RID for UBO");
  m_ubo_id = rids[0];
}

bool UniformBuffer::doLoad() {

  // 3. Create Uniform Buffer in all backends
  for (auto &rd : m_devices) {

    rd.get().createBuffer(
        BufferDesc{
            .size = m_buffer_size,
            .element_size = m_buffer_size,
            .usage = static_cast<uint32_t>(BufferUsage::UNIFORM_BUFFER),
            .is_host_visible = true, // Allow update from CPU
            .initial_data = m_packed_data.data(),
        },
        m_ubo_id);
  }

  return m_ubo_id.isValid();
}

bool UniformBuffer::doUnload() {
  for (auto &rd : m_devices) {
    rd.get().destroyBuffer(m_ubo_id);
  }
  m_ubo_id = RID::INVALID;
  return true;
}

void UniformBuffer::repack(const UniformSet &data) {
  // Repack current parameters
  m_layout.get()->packTo(data, m_packed_data.data());
  update(m_packed_data.data(), m_packed_data.size(), 0);
}

void UniformBuffer::update(const void *data, size_t size, size_t offset) {
  // Update buffer in GPU (via RenderDevice)
  for (auto &rd : m_devices) {
    rd.get().updateBufferRaw(m_ubo_id, offset, size, data);
  }
}

} // namespace ssme
