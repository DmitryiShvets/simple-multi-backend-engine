#include "pipeline_config_registry.h"
#include "../pipelines/default_pipeline.h"
#include "render_types.h"
namespace Render {

PipelineConfig
PipelineConfig::create(const std::string &name,
                       const std::function<void(PipelineDesc &)> &configure) {
  PipelineConfig config;
  config.name = name;
  if (configure) {
    configure(config.desc);
  }

  return config;
}

PipelineConfigRegistry::PipelineConfigRegistry(BackendType type)
    : m_backend_type(type) {}

void PipelineConfigRegistry::init() {
  DefaultPipeline::addConfigToRegistry(*this, m_backend_type);
}

PipelineConfiglHandle PipelineConfigRegistry::add(PipelineConfig &&material) {
  PipelineConfiglHandle id = m_next_id++;
  m_materials.emplace(id, std::move(material));
  m_name_to_id[m_materials.at(id).name] = id;
  return id;
}

PipelineConfiglHandle
PipelineConfigRegistry::add(const PipelineConfig &material) {
  PipelineConfiglHandle id = m_next_id++;
  m_materials.emplace(id, material);
  m_name_to_id[m_materials.at(id).name] = id;
  return id;
}

const PipelineConfig *
PipelineConfigRegistry::get(PipelineConfiglHandle id) const {
  auto it = m_materials.find(id);
  return it != m_materials.end() ? &it->second : nullptr;
}

PipelineConfig *PipelineConfigRegistry::get(PipelineConfiglHandle id) {
  auto it = m_materials.find(id);
  return it != m_materials.end() ? &it->second : nullptr;
}

const PipelineConfig *
PipelineConfigRegistry::getByName(const std::string &name) const {
  auto it = m_name_to_id.find(name);
  return it != m_name_to_id.end() ? &m_materials.at(it->second) : nullptr;
}

PipelineConfig *PipelineConfigRegistry::getByName(const std::string &name) {
  auto it = m_name_to_id.find(name);
  return it != m_name_to_id.end() ? &m_materials.at(it->second) : nullptr;
}

bool PipelineConfigRegistry::contains(PipelineConfiglHandle id) const {
  return m_materials.find(id) != m_materials.end();
}

bool PipelineConfigRegistry::containsByName(const std::string &name) const {
  return m_name_to_id.find(name) != m_name_to_id.end();
}

void PipelineConfigRegistry::remove(PipelineConfiglHandle id) {
  auto it = m_materials.find(id);
  if (it != m_materials.end()) {
    m_name_to_id.erase(it->second.name);
    m_materials.erase(it);
  }
}

const std::unordered_map<PipelineConfiglHandle, PipelineConfig> &
PipelineConfigRegistry::getAll() const {
  return m_materials;
}

const UniformLayout* PipelineConfigRegistry::getUniformLayout(const std::string& material_name) const {
  auto it = m_name_to_id.find(material_name);
  if (it != m_name_to_id.end()) {
    auto config_it = m_materials.find(it->second);
    if (config_it != m_materials.end()) {
      return &config_it->second.uniform_layout;
    }
  }
  return nullptr;
}

} // namespace Render
