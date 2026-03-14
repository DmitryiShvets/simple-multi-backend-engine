#pragma once
#include <glm/vec3.hpp>
#include <string>

class CUtils {
public:
  static float lerp(float a, float b, float t) { return a + (b - a) * t; }

  static float randomFloat(float min, float max);

  static glm::vec3 randomVec3F(float min, float max);

  static std::string readFile(const std::string &path);

  static std::vector<char> readFileChar(const std::string &filepath);
};
