#include "objects_utils.h"
#include "ecs/components/geometry_component.h"
#include "ecs/components/material_component.h"
#include "ecs/components/transform_component.h"
#include "vertex.h"

Core::Ecs::EntityHandle
createMesh(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
           const std::string &name, std::vector<Vertex> vertices,
           const std::string &mat_name) {
  auto circle = world.createEntity(name);
  world.addComponent<Core::Ecs::Component::Transform>(
      circle, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));

  world.addComponent<Core::Ecs::Component::Geometry>(circle, vertices);
  // world.addComponent<Core::Ecs::Component::DefaultMaterial>(circle, mat_name,
  //                                                           glm::vec3(0.0f));

  return circle;
}
