#include "shader.h"
#include "core/render_types.h"
#include "core/resource_types.h"
#include "core/rid.h"
#include "core/uniform_layout.h"
#include "core/uniform_value.h"
#include "render_device.h"
#include "utils/debug_assert.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace ssme {

ShaderModule::ShaderModule(std::string uuid, const VecRefRD &devices,
                           const ShaderModuleDesc &desc)
    : Resource(uuid, devices), m_desc(desc) {}

ShaderModule::~ShaderModule() {}

uint32_t ShaderModule::doPrepare() {
  // Copy data from desc that was prepared by loader
  m_vertex_requirements = m_desc.reflection.vertex_requirements;
  m_push_constants = m_desc.reflection.push_constants;
  m_push_constants_layouts = m_desc.reflection.push_constants_layouts;
  m_binding_layouts = m_desc.reflection.binding_layouts;
  m_ds_layouts = m_desc.reflection.ds_layouts;
  m_descriptor_set_count = m_desc.reflection.descriptor_set_count;

  m_required_components = m_desc.reflection.required_components;
  return m_required_components;
}

void ShaderModule::doSetup(const VecRID &rids) {
  debug_assert(rids.size() == m_required_components,
               "Material requires exactly 1 RID for UBO");
  m_module_rid = rids[0];
  uint32_t i = 1;
  debug_assert((rids.size() - 1) == m_ds_layouts.size(),
               "Dismatch of required RIDs and count of Descrtiptors in shader");
  for (auto &[key, val] : m_ds_layouts) {
    m_ds_layout_ids[key] = rids.at(i++);
  }
}

bool ShaderModule::doLoad() {
  for (auto &rd : m_devices) {
    // Create actual shader object in backend
    rd.get().createShaderModule(m_desc, m_module_rid);
    // Ask RenderDevice to create actual GPU scheme object
    for (auto &[key, val] : m_ds_layouts) {
      rd.get().createDescriptorLayout(val, m_ds_layout_ids[key]);
    }
  }

  return m_module_rid.isValid();
}

bool ShaderModule::doUnload() {
  for (auto &rd : m_devices) {
    rd.get().destroyBuffer(m_module_rid);
    for (int i = 0; i < m_descriptor_set_count; i++) {
      rd.get().destroyDescriptorLayout(m_ds_layout_ids[i]);
    }
  }

  for (int i = 0; i < m_descriptor_set_count; i++) {
    m_ds_layout_ids[i] = RID::INVALID;
  }
  m_module_rid = RID::INVALID;
  return true;
}

std::vector<RID> ShaderModule::getDescriptorLayouts() const {
  std::vector<RID> res;
  for (auto &[key, val] : m_ds_layout_ids) {
    res.push_back(val);
  }
  return res;
}

std::vector<PushConstantRange> ShaderModule::getPushConstants() const {
  std::vector<PushConstantRange> res;
  for (auto &[key, val] : m_push_constants) {
    res.push_back(val);
  }
  return res;
}

std::vector<VertexInputRequirement>
ShaderModule::getVertexInputRequirements() const {
  return m_vertex_requirements;
}

std::shared_ptr<UniformLayout> ShaderModule::getLayout(const std::string &block_name) const {
    std::shared_ptr<UniformLayout> layout = nullptr;

    if (m_binding_layouts.count(block_name)) {
      layout = m_binding_layouts.at(block_name);
    } else if (m_push_constants_layouts.count(block_name)) {
      layout = m_push_constants_layouts.at(block_name);
    }
    return layout;
}
/**
 * @brief Creates UniformSet containing all variables expected by this shader
 * block.
 * @param block_name Name of Uniform block (e.g., "MaterialBlock") or
 * Push constants.
 */
UniformSet ShaderModule::createUniformSet(const std::string &block_name) const {
  // 1. Find required layout (first in descriptors, then in push constants)
  std::shared_ptr<UniformLayout> layout = nullptr;

  if (m_binding_layouts.count(block_name)) {
    layout = m_binding_layouts.at(block_name);
  } else if (m_push_constants_layouts.count(block_name)) {
    layout = m_push_constants_layouts.at(block_name);
  }

  if (!layout) {
    debug_assert(false,
                 "Block '" + block_name + "' not found in shader reflection!");
    return UniformSet();
  }

  // 2. Create empty set
  UniformSet set;

  // 3. Fill it with "empty" values of required types based on layout.
  // This is critical: now UniformSet knows which names it MUST have.
  for (const auto &var : layout->getVariables()) {
    // Create default value of required type (zeros)
    set.set(var.name, UniformValue::createDefault(var.type));
  }

  return set;
}

bool ShaderModule::containsUniformBlock(const std::string &block_name) {
  return m_binding_layouts.count(block_name) > 0 ||
         m_push_constants_layouts.count(block_name) > 0;
}

} // namespace ssme
