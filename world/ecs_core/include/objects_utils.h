#pragma once

#include "flecs_world.h"
#include "vertex.h"
#include "world.h"

#include <glm/vec3.hpp>
#include <string>
#include <vector>


Core::Ecs::EntityHandle createMesh(
    Core::Ecs::World<Core::Ecs::FlecsWorldImpl> &world, const std::string& name,
    std::vector<Vertex> vertices, const std::string& mat_name);
