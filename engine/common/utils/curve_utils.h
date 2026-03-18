#pragma once
#include <glm/vec3.hpp>
#include <vector>

namespace ssme {

constexpr auto PI = 3.14;

class CurveUtils {
public:
  static std::vector<glm::vec3>
  createCircleGeometry(float x, float y, float radius, float segments = 16);

  static std::vector<glm::vec3>
  createCircleCoords(float x, float y, float radius, float segments = 16);

  static std::vector<glm::vec3> getSegmentSubdivision(const glm::vec3 &p1,
                                                      const glm::vec3 &p2,
                                                      float thickness);

  static std::vector<float>
  createLineGeometry(const std::vector<glm::vec3> &points, float thickness);

  static std::vector<glm::vec3>
  createRectangleCoords(float centerX, float centerY, float sideX, float sideY);

  static std::vector<glm::vec3>
  createPentagonCoords(float centerX, float centerY, float radius);
};

} // namespace ssme
