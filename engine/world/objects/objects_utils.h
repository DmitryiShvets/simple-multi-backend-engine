#pragma once

#include "ecs/entity.h"
#include "ecs/world.h"
#include "resource_manager.h"

#include <glm/vec3.hpp>
#include <string>

namespace ssme {
Entity _createSphere(World &world, ResourceManager &rm, const std::string &name,
                     float radius, int stacks, int sectors,
                     const glm::vec3 &pos, const std::string &mat_name);

Entity _createTriangle(World &world, ResourceManager &rm,
                      const std::string &name, const glm::vec3 &pos,
                      const std::string &mat_name = "default");

Entity _createQuad(World &world, ResourceManager &rm, const std::string &name,
                   const glm::vec3 &pos);


} // namespace ssme
