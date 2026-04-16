#include "scene.h"
#include "scene_view_system.h"
#include "resource_manager.h"
#include "ecs/world.h"

namespace ssme {

Scene::Scene(ResourceManager &rm)
    : m_world(std::make_unique<World>()),
      m_systems(std::make_unique<SystemManager>(*m_world)),
      m_resource_manager(rm) {
  // Register base logic

  // Register systems (e.g., TransformSystem)
  m_systems->addSystem<SceneViewSystem>(m_render_scene, m_resource_manager);
}

void Scene::update(TimeDelta dt) { m_systems->update(dt); }

} // namespace ssme
