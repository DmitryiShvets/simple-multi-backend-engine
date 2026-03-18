#pragma once
#include "core/gpu_types.h"
#include "core/resource_types.h"
#include "core/uniform_layout.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace ssme {
using PipelineConfiglHandle = uint64_t;

class PipelineConfigRegistry {
public:
  PipelineConfigRegistry(GpuBackend type);
  ~PipelineConfigRegistry() = default;

  void init();

  // Register a new material, returns its ID
  PipelineConfiglHandle add(PipelineConfig &&material);
  PipelineConfiglHandle add(const PipelineConfig &material);

  // Get material by ID (returns nullptr if not found)
  const PipelineConfig *get(PipelineConfiglHandle id) const;
  PipelineConfig *get(PipelineConfiglHandle id);

  // Get material by name (returns nullptr if not found)
  const PipelineConfig *getByName(const std::string &name) const;
  PipelineConfig *getByName(const std::string &name);

  // Check if material exists
  bool contains(PipelineConfiglHandle id) const;
  bool containsByName(const std::string &name) const;

  // Remove material by ID
  void remove(PipelineConfiglHandle id);

  // Get all materials
  const std::unordered_map<PipelineConfiglHandle, PipelineConfig> &
  getAll() const;

  // Get uniform layout for material (returns nullptr if not found)
  const UniformLayout* getUniformLayout(const std::string& material_name) const;

private:
  GpuBackend m_backend_type;
  std::unordered_map<PipelineConfiglHandle, PipelineConfig> m_materials;
  std::unordered_map<std::string, PipelineConfiglHandle> m_name_to_id;
  PipelineConfiglHandle m_next_id = 0;
};

} // namespace ssme
