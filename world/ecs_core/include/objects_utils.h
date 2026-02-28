#pragma once

#include "flecs_world.h"
#include "vertex.h"
#include "world.h"

#include <glm/vec3.hpp>
#include <string>
#include <vector>


Core::Ecs::EntityHandle createMesh(
    Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world, const std::string& name,
    std::vector<VertexN> vertices, const std::string& mat_name);

// Creates a sphere mesh using utils/sphere.h
Core::Ecs::EntityHandle createSphere(
    Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world, const std::string& name,
    float radius, int sphereStacks, int sphereSectors, const std::string& mat_name);
