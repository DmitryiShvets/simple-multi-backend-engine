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

  std::vector<SceneView> getSceneViews(size_t count) {
      std::vector<SceneView> views(count, m_render_scene);
      for (size_t i = 0; i < count && i < m_view_cameras.size(); ++i) {
        views[i].camera = m_view_cameras[i];
      }
      return views;
  }

  void setCamera(std::shared_ptr<Camera> cam) { m_render_scene.camera = std::move(cam); } // fallback
  void setCamera(size_t view_index, std::shared_ptr<Camera> cam) {
    if (m_view_cameras.size() <= view_index) {
      m_view_cameras.resize(view_index + 1);
    }
    m_view_cameras[view_index] = std::move(cam);
  }

private:
  std::unique_ptr<World> m_world;
  std::unique_ptr<SystemManager> m_systems;
  ResourceManager &m_resource_manager;
  SceneView m_render_scene;
  std::vector<std::shared_ptr<Camera>> m_view_cameras;
};

} // namespace ssme
