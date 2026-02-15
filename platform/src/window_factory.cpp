#include "window_factory.h"
#include "glfw_main_window.h"
#include "i_gpu_context_strategy.h"

#include <stdexcept>

namespace Window {
std::unique_ptr<IMainWindow>
WindowFactory::create(API api, const WindowConfig &config,
                      const IGpuContextStrategy &gpuStrategy) {

  switch (api) {
  case API::GLFW: {
    auto window = std::make_unique<GLFWMainWindow>(config);
    window->init(gpuStrategy);
    return window;
  }
  default:
    throw std::runtime_error("Unknown API");
  }
}

} // namespace Window
