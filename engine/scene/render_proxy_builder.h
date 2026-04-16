#pragma once
#include "objects/objects_uniforms.h"

namespace ssme {

class Entity;
class World;
class ResourceManager;

class RenderProxyBuilder {
public:
  RenderProxyBuilder(ResourceManager &rm);
  void buildProxy(Entity &entity);
  void updateProxy(const Entity &entity);
  void ensureConsistency(const Entity &entity);

private:
  ResourceManager &m_rm;
  ObjectUniformsPacker m_packer;
};
} // namespace ssme
