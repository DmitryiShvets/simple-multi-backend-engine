#pragma once
#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
struct SphereGeometry {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> text_coords;
  std::vector<uint32_t> indices;
  std::vector<uint32_t> line_indices;
};

SphereGeometry getSphere3D(float radius, uint64_t sector_count, uint64_t stack_count);
