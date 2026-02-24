#pragma once
#include "concepts.h"
#include "ecs/components/runtime_component.h"
#include "ecs/components/transform_component.h"
#include "render_device.h"
#include "uniforms.h"

namespace Core::Ecs::System {

// RuntimeUpdateSystem - updates runtime GPU resources every frame
// Called before SceneViewSystem::run() to ensure all per-object data is up to date
// Handles both Vulkan and OpenGL backends in a single system
template <typename WorldType>
  requires EcsWorld<WorldType>
class RuntimeUpdateSystem {
public:
  RuntimeUpdateSystem(WorldType &world, Render::Device &vk_device, Render::Device &gl_device)
      : m_world(world), m_vk_device(vk_device), m_gl_device(gl_device),
        m_query(world.template createQuery<Component::Transform,
                                           Component::VkRuntime,
                                           Component::GlRuntime>()) {}

  void update() {
    m_query.each([this](EntityHandle /*entity*/,
                        Component::Transform &transform,
                        Component::VkRuntime &vk_rt,
                        Component::GlRuntime &gl_rt) {
      if (vk_rt.visible) {
        // Update per-object model matrix (used for push constants in default material)
        vk_rt.data_id.model_matrix = transform.getModelMatrix();
      }
      if (gl_rt.visible) {
        // Update per-object model matrix (used for push constants in default material)
        gl_rt.data_id.model_matrix = transform.getModelMatrix();
      }
    });
  }

private:
  WorldType &m_world;
  Render::Device &m_vk_device;
  Render::Device &m_gl_device;
  decltype(m_world.template createQuery<Component::Transform,
                                        Component::VkRuntime,
                                        Component::GlRuntime>()) m_query;
};

} // namespace Core::Ecs::System
