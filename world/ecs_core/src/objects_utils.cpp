#include "objects_utils.h"
#include "ecs/components/geometry_component.h"
#include "ecs/components/material_component.h"
#include "ecs/components/transform_component.h"
#include "vertex.h"
#include "sphere.h"
#include <glm/gtc/matrix_transform.hpp>

Core::Ecs::EntityHandle
createMesh(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
           const std::string &name, std::vector<VertexN> vertices,
           const std::string &mat_name) {
  auto entity = world.createEntity(name);
  world.addComponent<Core::Ecs::Component::Transform>(
      entity, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));

  world.addComponent<Core::Ecs::Component::Geometry>(entity, vertices);

  return entity;
}

Core::Ecs::EntityHandle
createSphere(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
             const std::string &name, float radius, int stacks, int sectors,
             const std::string &mat_name) {
  auto sphere_entity = world.createEntity(name);
  world.addComponent<Core::Ecs::Component::Transform>(
      sphere_entity, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));

  // Generate sphere using utils/sphere.h
  SphereGeometry sphere_geom = getSphere3D(radius, sectors, stacks);

  // Convert indexed geometry to non-indexed (duplicate vertices for each index)
  std::vector<VertexN> vertices;
  vertices.reserve(sphere_geom.indices.size());

  for (size_t i = 0; i < sphere_geom.indices.size(); ++i) {
    uint32_t index = sphere_geom.indices[i];
    vertices.push_back({
        .position = sphere_geom.positions[index],
        .normal = sphere_geom.normals[index]
    });
  }

  world.addComponent<Core::Ecs::Component::Geometry>(sphere_entity, vertices);

  return sphere_entity;
}
