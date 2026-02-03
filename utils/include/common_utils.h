#pragma once
#include <string>
#include <glm/vec3.hpp>

class CUtils
{
public:

	static float randomFloat(float min, float max);

	static glm::vec3 randomVec3F(float min, float max);

	static std::string readFile(const std::string& path);
};

