#include "objects_uniforms.h"
#include "ecs/components/transform_component.h"
#include "utils/debug_assert.h"
#include <utility>

namespace ssme {

ObjectUniformRegistry::ObjectUniformRegistry() { registerCommonGetters(); }

void ObjectUniformRegistry::registerCommonGetters() {
  // =========================================================================
  // Model matrix
  // =========================================================================
  ObjectUniformRegistry::addGetter(
      "u_model_mat", [](Entity &entity) -> UniformValue {
        glm::mat4 model_mat =
            entity.get<TransformComponent>()->getModelMatrix();
        return UniformValue(model_mat);
      });
  // =========================================================================
  // Normal matrix
  // =========================================================================
  ObjectUniformRegistry::addGetter(
      "u_normal_mat", [](Entity &entity) -> UniformValue {
        glm::mat4 model_mat =
            entity.get<TransformComponent>()->getModelMatrix();
        glm::mat3 normal_mat =
            glm::transpose(glm::inverse(glm::mat3(model_mat)));
        return UniformValue(normal_mat);
      });
  // =========================================================================
  // Misc
  // =========================================================================
  ObjectUniformRegistry::addGetter("u_test_value",
                                   [](Entity &entity) -> UniformValue {
                                     float test_val = 777.0f;
                                     return UniformValue(test_val);
                                   });
}

void ObjectUniformsPacker::fillUniformSet(const Entity &ent, UniformSet &u_set,
                                          const UniformLayout &layout) {
  auto variables = layout.getVariables();
  for (auto &var : variables) {
    bool has_getter = m_registry.hasGetter(var.name);
    debug_assert(has_getter, "Objects Unirom registry hasn't user unifrom");
    if (!has_getter)
      continue;
    u_set.set(var.name, std::move(m_registry.get(ent, var.name)));
  }
}
} // namespace ssme
