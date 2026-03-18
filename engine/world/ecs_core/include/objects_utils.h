#pragma once

#include "flecs_world.h"
#include "vertex.h"
#include "world.h"

#include <glm/vec3.hpp>
#include <string>
#include <vector>

Core::Ecs::EntityHandle
createMesh(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
           const std::string &name, std::vector<VertexN> vertices,
           const std::string &mat_name);

// Creates a sphere mesh using utils/sphere.h
Core::Ecs::EntityHandle
createSphere(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
             const std::string &name, float radius, int sphereStacks,
             int sphereSectors, const std::string &mat_name);

// Creates a triangle with Vertex (position only) - uses default material
Core::Ecs::EntityHandle
createTriangle(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
               const std::string &name, const glm::vec3 &pos,
               const std::string &mat_name = "default");

// Creates a triangle with VertexN (position + normal) - uses ads material
Core::Ecs::EntityHandle
createTriangleN(Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world,
                const std::string &name, const glm::vec3 &pos,
                const std::string &mat_name = "ads");
