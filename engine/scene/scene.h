#pragma once

#include "core/time.h"
#include "scene_view.h"
#include "ecs/system_manager.h"

#include <memory>

namespace ssme {
class World;
class SystemManager;
class ResourceManager;

class Scene {
public:
  Scene(ResourceManager &rm);

  void update(TimeDelta dt);

  World &getWorld() { return *m_world; }

  std::vector<SceneView> getSceneViews() {
      // todo: fix this
    return {m_render_scene, m_render_scene, m_render_scene};
  }

private:
  std::unique_ptr<World> m_world;
  std::unique_ptr<SystemManager> m_systems;
  ResourceManager &m_resource_manager;
  SceneView m_render_scene;
};

} // namespace ssme
