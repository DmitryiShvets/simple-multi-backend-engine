#pragma once

/**
 * @brief Uniform Factory Registry
 *
 * Registers factories that convert material data → UniformSet.
 *
 * Architecture:
 * - Each material type registers a factory function at startup
 * - RuntimeInitSystem uses the registry to create uniforms without knowing
 * material types
 *
 * To add a new material:
 * 1. Define material params struct with material_type_name
 * 2. Add factory registration in registerUniformFactories()
 * 3. Define vertex type with getLayout() method
 * 4. Create pipeline config with vertex_layout set
 */

#include "core/uniform_layout.h"
#include "core/uniform_set.h"
#include "core/uniform_value.h"
#include "ecs/entity.h"

#include <functional>
#include <set>
#include <string>
#include <unordered_map>

namespace ssme {

class World;
/**
 * @brief Factory function type for converting material data to UniformSet
 *
 * Takes material component data and returns UniformSet for GPU.
 * Does NOT create materials or buffers - only converts data → uniforms.
 */
using ObjectUniformGetter =
    std::function<UniformValue(Entity &entity)>;

/**
 * @brief Registry of uniform factories
 *
 * Maps material type names to their uniform factories.
 * Each material type registers its factory at startup.
 */
class ObjectUniformRegistry {
public:
  ObjectUniformRegistry();

  /**
   * @brief Register a factory for a material type
   */
  void addGetter(const std::string &name,
                               ObjectUniformGetter getter) {
    m_getters[name] = std::move(getter);
  }

  /**
   * @brief Get factory for a material type
   */
  UniformValue get(Entity e, const std::string &name) const {
    auto it = m_getters.find(name);
    return it != m_getters.end() ? it->second(e) : UniformValue();
  }

  /**
   * @brief Check if a material type is registered
   */
  bool hasGetter(const std::string &name) const {
    return supported_uniforms.contains(name);
  }

  const std::set<std::string> supported_uniforms = {
      "u_model_mat",
      "u_normal_mat",
      "u_test_value",
  };

private:
  void registerCommonGetters();
  std::unordered_map<std::string, ObjectUniformGetter> m_getters;
};

class ObjectUniformsPacker {
public:
  void fillUniformSet(const Entity &ent, UniformSet& set, const UniformLayout& layout);

private:
  ObjectUniformRegistry m_registry;
};

} // namespace ssme
