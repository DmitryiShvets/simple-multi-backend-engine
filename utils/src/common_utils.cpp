#include "common_utils.h"

#include <random>
#include <chrono>
#include <iostream>
#include <fstream>

float CUtils::randomFloat(float min, float max) {
	static auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	static std::mt19937 rng(static_cast<unsigned>(seed));
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(rng);
}

std::string CUtils::readFile(const std::string& path) {
	std::ifstream input_file(path);
	if (!input_file.is_open()) {
		std::cerr << "Could not open the file - '"
			<< path << "'" << std::endl;
		exit(EXIT_FAILURE);
	}
	return std::string{ (std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>() };
}

glm::vec3 CUtils::randomVec3F(float min, float max)
{
	return glm::vec3(CUtils::randomFloat(min, max), CUtils::randomFloat(min, max), CUtils::randomFloat(min, max));
}
