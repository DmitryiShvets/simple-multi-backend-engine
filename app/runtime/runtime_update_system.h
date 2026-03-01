#pragma once
#include "concepts.h"
#include "ecs/components/runtime_component.h"
#include "ecs/components/transform_component.h"
#include "render_device.h"
#include "uniforms.h"

namespace Core::Ecs::System {

// RuntimeUpdateSystem - updates runtime GPU resources every frame
// Called before SceneViewSystem::run() to ensure all per-object data is up to
// date Handles both Vulkan and OpenGL backends in a single system
template <typename WorldType>
  requires EcsWorld<WorldType>
class RuntimeUpdateSystem {
public:
  RuntimeUpdateSystem(WorldType &world, Render::Device &vk_device,
                      Render::Device &gl_device)
      : m_world(world), m_vk_device(vk_device), m_gl_device(gl_device),
        m_query(world.template createQuery<Component::Transform,
                                           Component::VkRuntime,
                                           Component::GlRuntime>()) {}

  void update() {
    m_query.each([this](
                     EntityHandle /*entity*/, Component::Transform &transform,
                     Component::VkRuntime &vk_rt, Component::GlRuntime &gl_rt) {

      // Update per-object model matrix and normal matrix
      glm::mat4 model_mat = transform.getModelMatrix();
      glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(model_mat)));

      vk_rt.model_matrix = model_mat;
      gl_rt.model_matrix = model_mat;

      // Skip if object uniforms are not used (e.g., default material)
      if (!vk_rt.obj_uniform_id.isValid() && !gl_rt.obj_uniform_id.isValid()) {
        return;
      }
      // Convert to std140 layout for GPU
      Uniforms::ObjectUniforms obj_uniforms{.model_matrix = model_mat,
                                            .normal_matrix = normal_mat};
      auto packed = Uniforms::ObjectUniformsStd140::from(obj_uniforms);

      if (vk_rt.visible && vk_rt.obj_uniform_id.isValid()) {
        m_vk_device.updateBufferRaw(vk_rt.obj_uniform_id, 0, sizeof(packed),
                                    &packed);
      }
      if (gl_rt.visible && gl_rt.obj_uniform_id.isValid()) {
        m_gl_device.updateBufferRaw(gl_rt.obj_uniform_id, 0, sizeof(packed),
                                    &packed);
      }
    });
  }

private:
  WorldType &m_world;
  Render::Device &m_vk_device;
  Render::Device &m_gl_device;
  decltype(m_world
               .template createQuery<Component::Transform, Component::VkRuntime,
                                     Component::GlRuntime>()) m_query;
};

} // namespace Core::Ecs::System
