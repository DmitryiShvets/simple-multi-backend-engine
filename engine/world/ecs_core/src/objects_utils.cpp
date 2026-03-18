#include "objects_utils.h"
#include "ecs/components/geometry_component.h"
#include "ecs/components/material_component.h"
#include "ecs/components/transform_component.h"
#include "sphere.h"
#include "vertex.h"
#include <glm/gtc/matrix_transform.hpp>

Core::Ecs::EntityHandle
createMesh(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
           const std::string &name, std::vector<VertexN> vertices,
           const std::string &mat_name) {
  auto entity = world.createEntity(name);
  world.addComponent<Core::Ecs::Component::Transform>(
      entity, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));

  // Use GeometryN for VertexN vertices
  Core::Ecs::Component::GeometryN geom;
  geom.vertices = std::move(vertices);
  world.addComponent<Core::Ecs::Component::GeometryN>(entity, std::move(geom));

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
    vertices.push_back({.position = sphere_geom.positions[index],
                        .normal = sphere_geom.normals[index]});
  }

  world.addComponent<Core::Ecs::Component::GeometryN>(sphere_entity,
                                                      std::move(vertices));

  return sphere_entity;
}

Core::Ecs::EntityHandle
createTriangle(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
               const std::string &name, const glm::vec3 &pos,
               const std::string &mat_name) {
  auto entity = world.createEntity(name);
  world.addComponent<Core::Ecs::Component::Transform>(
      entity, pos, glm::vec3(0.0f), glm::vec3(1.0f));

  // Triangle with position only (Vertex)
  std::vector<Vertex> vertices = {
      {{0.0f, 0.5f, 0.0f}},   // Top
      {{-0.5f, -0.5f, 0.0f}}, // Bottom left
      {{0.5f, -0.5f, 0.0f}}   // Bottom right
  };

  world.addComponent<Core::Ecs::Component::Geometry>(entity,
                                                     std::move(vertices));

  return entity;
}

Core::Ecs::EntityHandle
createTriangleN(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
                const std::string &name, const glm::vec3 &pos,
                const std::string &mat_name) {
  auto entity = world.createEntity(name);
  world.addComponent<Core::Ecs::Component::Transform>(
      entity, pos, glm::vec3(0.0f), glm::vec3(1.0f));

  // Triangle with position + normal (VertexN)
  std::vector<VertexN> vertices = {
      {{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},   // Top
      {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Bottom left
      {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Bottom right
  };

  world.addComponent<Core::Ecs::Component::GeometryN>(entity,
                                                      std::move(vertices));

  return entity;
}
