#pragma once

#include "core/resource_types.h"
#include "core/uniform_layout.h"
#include "core/uniform_set.h"
#include "resource.h"
#include <cstdint>
#include <memory>
#include <string>

namespace ssme {

/**
 * @brief Uniform Buffer description
 *
 * UniformBuffer is a resource that stores data for transfer to GPU.
 * Data is packed according to std140 for shader compatibility.
 */
struct UniformBlockDesc {
  std::string name;          // For debugging
  uint64_t size = 0;         // Size in bytes
  std::vector<uint8_t> data; // Raw data (std140)
  std::shared_ptr<UniformLayout> layout;
};

class UniformBuffer : public Resource {
public:
  using ParamsType = UniformBlockDesc;
  static constexpr ResourceId ID = ResourceId::UNIFORM_BUFFER;
  static constexpr uint32_t COMPONENTS = 1; // UBO

  explicit UniformBuffer(std::string id, const VecRefRD &devices,
                        const UniformBlockDesc &desc);
  ~UniformBuffer() override;

  // --- Resource Interface ---
  uint32_t doPrepare() override;
  void doSetup(const VecRID &rids) override;
  bool doLoad() override;
  bool doUnload() override;

  RID getUbo() const { return m_ubo_id; }
  const uint8_t *getData() const { return m_packed_data.data(); }
  size_t getDataSize() const { return m_packed_data.size(); }

  void repack(const UniformSet &data);
  /**
   * @brief Update data (partially or fully)
   */
  void update(const void *data, size_t size, size_t offset = 0);

private:
  RID m_ubo_id = RID::INVALID;

  std::string m_label;
  uint64_t m_buffer_size = 0;
  std::vector<uint8_t> m_packed_data; // Cached std140 bytes
  std::shared_ptr<UniformLayout> m_layout;
  bool m_dirty = false;
};
} // namespace ssme
