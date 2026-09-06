#include "objects/objects_utils.h"
#include "core/render_types.h"
#include "core/resource_types.h"
#include "ecs/components/material_component.h"
#include "ecs/components/mesh_component.h"
#include "ecs/components/transform_component.h"
#include "geometry_generator.h"
#include "resource_handle.h"
#include "resources/material.h"
#include "resources/mesh.h"
#include <glm/gtc/matrix_transform.hpp>

namespace ssme {

Entity _createSphere(World &world, ResourceManager &rm, const std::string &name,
                     float radius, int stacks, int sectors,
                     const glm::vec3 &pos, const std::string &mat_name) {
  Entity obj = world.createEntity(name);
  obj.add<TransformComponent>(pos, glm::vec3{-90,0,0}, glm::vec3{1,1,1});

  MeshDesc desc = GeometryGenerator::createSphere(radius, sectors, stacks);
  ResourceHandle mesh = rm.load<Mesh>("sphere", desc);
  ResourceHandle mat = rm.load<Material>(mat_name);

  MaterialComponent mat_comp(mat);
  MeshComponent mesh_comp(mesh);
  world.addComponent(obj, std::move(mesh_comp));
  world.addComponent(obj, std::move(mat_comp));

  return obj;
}

Entity _createTriangle(World &world, ResourceManager &rm,
                       const std::string &name, const glm::vec3 &pos,
                       const std::string &mat_name) {
  Entity obj = world.createEntity(name);
  obj.add<TransformComponent>(pos);
  MeshDesc desc = GeometryGenerator::createTriangle();
  ResourceHandle mesh = rm.load<Mesh>("triangle", desc);
  ResourceHandle mat = rm.load<Material>("default.json");

  MaterialComponent mat_comp(mat);
  MeshComponent mesh_comp(mesh);
  world.addComponent(obj, std::move(mesh_comp));
  world.addComponent(obj, std::move(mat_comp));
  return obj;
}

Entity _createQuad(World &world, ResourceManager &rm, const std::string &name,
                   const glm::vec3 &pos) {
  Entity obj = world.createEntity(name);
  obj.add<TransformComponent>(pos);
  MeshDesc desc = GeometryGenerator::createQuad();
  ResourceHandle mesh = rm.load<Mesh>("quad", desc);
  ResourceHandle mat = rm.load<Material>("textured.json");
  MaterialComponent mat_comp(mat);
  MeshComponent mesh_comp(mesh);
  world.addComponent(obj, std::move(mesh_comp));
  world.addComponent(obj, std::move(mat_comp));
  return obj;
}

} // namespace ssme
