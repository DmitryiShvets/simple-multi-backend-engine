#pragma once
#include "i_main_window.h"

#include <memory>

namespace Window {
enum class API { GLFW };
class IGpuContextStrategy;

class WindowFactory {
public:
  static std::unique_ptr<IMainWindow>
  create(API api, const WindowConfig &config,
         const IGpuContextStrategy &gpuStrategy);
};
} // namespace Window
