#pragma once

#include "core/resource_types.h"
#include "resource.h"
#include <cstdint>
#include <string>

namespace ssme {

/**
 * @brief Descriptor Set Layout resource
 *
 * Creates descriptor layout for pipeline
 */
class DescriptorSetLayout : public Resource {
public:
  using ParamsType = DescriptorLayout;
  static constexpr ResourceId ID = ResourceId::DESCRIPTOR_SET_LAYOUT;
  static constexpr uint32_t COMPONENTS = 1;

  explicit DescriptorSetLayout(std::string id, const VecRefRD &devices,
                               const DescriptorLayout &desc);
  ~DescriptorSetLayout() override;

  uint32_t doPrepare() override;
  void doSetup(const VecRID &rids) override;
  bool doLoad() override;
  bool doUnload() override;

  RID getLayoutId() const { return m_layout_id; }

private:
  DescriptorLayout m_layout_desc;
  RID m_layout_id = RID::INVALID;
};

/**
 * @brief Descriptor Set resource
 *
 * Allocates and configures descriptor set from pool
 */
class DescriptorSet : public Resource {
public:
  using ParamsType = DescriptorDesc;
  static constexpr ResourceId ID = ResourceId::DESCRIPTOR_SET;
  static constexpr uint32_t COMPONENTS = 1;

  explicit DescriptorSet(std::string id, const VecRefRD &devices,
                         const DescriptorDesc &desc);
  ~DescriptorSet() override;

  uint32_t doPrepare() override;
  void doSetup(const VecRID &rids) override;
  bool doLoad() override;
  bool doUnload() override;

  RID getDescriptorSetId() const { return m_ds_id; }
  RID getLayoutId() const { return m_layout_id; }

private:
  DescriptorDesc m_ds_desc;
  RID m_ds_id = RID::INVALID;
  RID m_layout_id = RID::INVALID;
};

} // namespace ssme
