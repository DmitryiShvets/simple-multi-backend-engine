#pragma once
#include "i_gpu_context_strategy.h"
#include "i_main_window.h"

#include <memory>

namespace Window {
enum class API { GLFW };

class WindowFactory {
public:
  static std::unique_ptr<IMainWindow>
  create(API api, const WindowConfig &config,
         const IGpuContextStrategy &contextStrategy);
};
} // namespace Window
