#pragma once
#include "concepts.h"
#include "ecs/components/render_component.h"
#include "ecs/components/transform_component.h"
#include "scene_view.h"
#include <vector>

namespace Core::Ecs::System {

template <typename WorldType>
  requires EcsWorld<WorldType>
class RenderSystem {
public:
  RenderSystem(WorldType &world)
      : m_world(world),
        m_query(world.template createQuery<const Component::Transform,
                                           const Component::Renderable>()) {}

  SceneView createSceneView() {
    SceneView sceneView;
    m_query.each([&sceneView](EntityHandle /*entity*/,
                              const Component::Transform & /*transform*/,
                              const Component::Renderable &e) {
      if (e.visible) {

        sceneView.renderables.push_back(SceneView::Renderable{
            .mesh_id = e.mesh_id,
            .shader_id = e.shader_id,
            .color = e.color,
            .visible = e.visible,
        });
      }
    });
    return sceneView;
  }

private:
  WorldType &m_world;
  decltype(m_world.template createQuery<const Component::Transform,
                                        const Component::Renderable>()) m_query;
};

} // namespace Core::Ecs::System
