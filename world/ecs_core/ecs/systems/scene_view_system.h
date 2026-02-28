#pragma once
#include "concepts.h"
#include "ecs/components/transform_component.h"
#include "scene_view.h"
#include <vector>

namespace Core::Ecs::System {

template <typename WorldType, typename RuntimeComponent>
  requires EcsWorld<WorldType>
class SceneViewSystem {
public:
  SceneViewSystem(WorldType &world)
      : m_world(world),
        m_query(world.template createQuery<const Component::Transform,
                                           const RuntimeComponent>()) {}

  SceneView run() {
    SceneView sceneView;
    m_query.each([&sceneView](EntityHandle /*entity*/,
                              const Component::Transform & transform,
                              const RuntimeComponent &e) {
      if (e.visible) {
        sceneView.opaque_objects.push_back(SceneView::MeshRenderable{
            .geometry_id = e.geometry_id,
            .material_id = e.material_id,
            .obj_uniform_id = e.obj_uniform_id,
            .obj_uniform_ds = e.obj_uniform_ds,
            .model_matrix = e.model_matrix,
        });
      }
    });
    return sceneView;
  }

private:
  WorldType &m_world;
  decltype(m_world.template createQuery<const Component::Transform,
                                        const RuntimeComponent>()) m_query;
};

} // namespace Core::Ecs::System
