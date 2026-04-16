#pragma once
#include "ecs/system.h"
#include "ecs/world.h"
#include "render_proxy_builder.h"
#include "scene_view.h"

namespace ssme {
class ResourceManager;
class SceneViewSystem : public System {
public:
  SceneViewSystem(SceneView &view, ResourceManager &rm)
      : m_view(view), m_rm(rm), m_proxy_builder(rm) {}

  const char *getName() const override { return "SceneViewSystem"; }

  void awake(World &world) override;
  void update(TimeDelta dt, World &world) override;

private:
  void build(EntityID id, World &w);
  void rem(EntityID id, World &w);
  SceneView &m_view;
  ResourceManager &m_rm;
  RenderProxyBuilder m_proxy_builder;
  EntityID m_add_tag =
      0; // ID of our dynamic tag for adding proxy to scene
  EntityID m_del_tag = 0; // ID of our tag for proxy removal from scene
};

} // namespace ssme
