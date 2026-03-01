#pragma once
#include <glm/vec3.hpp>
#include <string>

namespace Core::MaterialParams {

// ============================================================================
// Material parameters (instance-level)
// Stored in ECS component for EACH object (instance-level)
// ============================================================================

struct DefaultMaterial {
  glm::vec3 color = glm::vec3(1.0f);

  static constexpr const char *material_type_name = "default";
};

struct AdsMaterial {
  glm::vec3 color = glm::vec3(1.0f);

  static constexpr const char *material_type_name = "ads";
};

struct PbrMaterialParams {
  std::string tex_albedo_path;
  std::string tex_metallic_path;
  std::string tex_roughness_path;
  float metallic_factor = 1.0f;
  float roughness_factor = 1.0f;

  static constexpr const char *material_type_name = "pbr";
};

} // namespace Core::MaterialParams
