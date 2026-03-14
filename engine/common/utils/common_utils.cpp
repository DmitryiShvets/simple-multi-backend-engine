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

std::vector<char> CUtils::readFileChar(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + filepath);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

glm::vec3 CUtils::randomVec3F(float min, float max)
{
	return glm::vec3(CUtils::randomFloat(min, max), CUtils::randomFloat(min, max), CUtils::randomFloat(min, max));
}
