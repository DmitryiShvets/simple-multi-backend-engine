/**
 * @brief New application entry point using ssme engine
 *
 * This is the new application that uses the ssme engine
 * with proper abstraction layers.
 */

#include "engine.h"
#include <cstdlib>
#include <glm/ext/vector_float3.hpp>
#include <iostream>

// Constants
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
int main() {

  try {
    ssme::Engine engine;
    engine.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);
    engine.createSphere("1", 0.5f, 32, 32, glm::vec3{0.0f, 0.0f, -1.0f}, "ads.json");
    engine.createSphere("2", 0.5f, 32, 32, glm::vec3{1.f, 0.0f, -1.0f}, "ads.json");
    engine.createSphere("3", 0.5f, 32, 32, glm::vec3{1.f, 1.0f, -1.0f}, "textured.json");
    engine.createQuad("quad", glm::vec3{-1.f, -1.0f, -1.0f});
    engine.createTriangle("4", glm::vec3{-1.f, 1.0f, -1.0f});
    engine.run();
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
// git clone
// git init submodules
// cp patches/imgui.CMakeLists.txt external/imgui/CMakeLists.txt
