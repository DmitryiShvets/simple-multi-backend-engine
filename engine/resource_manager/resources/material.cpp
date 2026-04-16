#include "material.h"
#include "render_device.h"
#include "utils/debug_assert.h"

namespace ssme {

Material::Material(std::string uuid, const VecRefRD &devices,
                   const MaterialParams &desc)
    : Resource(uuid, devices), m_vert_shader(std::move(desc.vert_shader)),
      m_frag_shader(std::move(desc.frag_shader)),
      m_blocks_data(std::move(desc.blocks_data)),
      m_textures(std::move(desc.textures)),
      m_uniforms(std::move(desc.uniforms)) {}

Material::~Material() {
  // doUnload();
}

uint32_t Material::doPrepare() {
  m_requred_components = m_uniforms.size() * 2;
  m_ubo_ds_layouts.resize(m_uniforms.size());
  m_ubo_ds.resize(m_uniforms.size());
  return m_requred_components;
}

void Material::doSetup(const VecRID &rids) {
  debug_assert(rids.size() == m_requred_components,
               "Material requires exactly 1 RID for UBO");
  for (int i = 0, j = 0; i < m_requred_components / 2; i++, j += 2) {
    m_ubo_ds_layouts[i] = rids[j];
    m_ubo_ds[i] = rids[j + 1];
  }
}

bool Material::doLoad() {

  for (auto &rd : m_devices) {
    int i = 0;
    for (auto &uni : m_uniforms) {
      DescriptorLayout ds_layout_desc;
      ds_layout_desc.bindings.push_back({
          .binding = 0,
          .type = DescriptorType::UNIFORM_BUFFER,
          .count = 1,
          .stages = static_cast<uint32_t>(ShaderStage::VERTEX),
      });
      auto uniform_ds_layout =
          rd.get().createDescriptorLayout(ds_layout_desc, m_ubo_ds_layouts[i]);
      DescriptorDesc desc{
          .layout_id = uniform_ds_layout,
          .uniform_buffers = std::vector<RID>{uni.second->getUbo()},
      };
      rd.get().createDescriptor(desc, m_ubo_ds[i]);
    }
    i++;
  }
  return true;
}

bool Material::doUnload() {

  for (auto &rd : m_devices) {
    int i = 0;
    for (auto &uni : m_uniforms) {
      rd.get().destroyDescriptor(m_ubo_ds[i]);
      rd.get().destroyDescriptorLayout(m_ubo_ds_layouts[i]);
    }
    i++;
  }
  return true;
}

std::map<std::string, RID> Material::getTextures() const {
  std::map<std::string, RID> result;
  for (auto const &[name, handle] : m_textures) {
    if (auto *tex = handle.get()) {
      result[name] = tex->getTexture();
    }
  }
  return result;
}

UniformSet &Material::block(const std::string &block_name) {
  return m_blocks_data[block_name];
}

} // namespace ssme
