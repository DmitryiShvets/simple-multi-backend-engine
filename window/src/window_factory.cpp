#include "window_factory.h"
#include "glfw_main_window.h"
#include <stdexcept>

namespace Window {
std::unique_ptr<IMainWindow>
WindowFactory::create(API api, const WindowConfig &config,
                      const IGpuContextStrategy &contextStrategy) {

  switch (api) {
  case API::GLFW: {
    auto window = std::make_unique<GLFWMainWindow>(config);
    window->initialize(contextStrategy);
    return window;
  }
  default:
    throw std::runtime_error("Unknown API");
  }
}

} // namespace Window
