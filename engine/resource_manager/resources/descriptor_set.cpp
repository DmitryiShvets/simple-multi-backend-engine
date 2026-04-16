#include "descriptor_set.h"
#include "render_device.h"
#include "utils/debug_assert.h"

namespace ssme {

DescriptorSetLayout::DescriptorSetLayout(std::string id,
                                         const VecRefRD &devices,
                                         const DescriptorLayout &desc)
    : Resource(id, devices), m_layout_desc(std::move(desc)) {}

DescriptorSetLayout::~DescriptorSetLayout() {}

uint32_t DescriptorSetLayout::doPrepare() { return COMPONENTS; }

void DescriptorSetLayout::doSetup(const VecRID &rids) {
  debug_assert(rids.size() == COMPONENTS,
               "The number of RIDs and the number of components must match");
  m_layout_id = rids[0];
}

bool DescriptorSetLayout::doLoad() {
  for (auto &rd : m_devices) {
    rd.get().createDescriptorLayout(m_layout_desc, m_layout_id);
  }
  return true;
}

bool DescriptorSetLayout::doUnload() {
  for (auto &rd : m_devices) {
    rd.get().destroyDescriptorLayout(m_layout_id);
  }
  m_layout_id = RID::INVALID;
  return true;
}

DescriptorSet::DescriptorSet(std::string id, const VecRefRD &devices,
                             const DescriptorDesc &desc)
    : Resource(id, devices), m_layout_id(desc.layout_id),
      m_ds_desc(std::move(desc)) {}

DescriptorSet::~DescriptorSet() {}

uint32_t DescriptorSet::doPrepare() { return COMPONENTS; }

void DescriptorSet::doSetup(const VecRID &rids) {
  debug_assert(rids.size() == COMPONENTS,
               "The number of RIDs and the number of components must match");
  m_ds_id = rids[0];
}

bool DescriptorSet::doLoad() {
  for (auto &rd : m_devices) {
    rd.get().createDescriptor(m_ds_desc, m_ds_id);
  }
  return true;
}

bool DescriptorSet::doUnload() {
  for (auto &rd : m_devices) {
    rd.get().destroyDescriptor(m_ds_id);
  }
  m_ds_id = RID::INVALID;
  return true;
}

} // namespace ssme
