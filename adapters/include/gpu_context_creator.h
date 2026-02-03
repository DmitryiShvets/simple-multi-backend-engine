#pragma once
#include "i_gpu_context_strategy.h"

namespace Window {

class OpenGLGpuContextCreator : public IGpuContextStrategy {
public:
  void prepareWindowCreationHints() const override;
  bool createContext(void *window) const override;
};
class VulkanGpuContextCreator : public IGpuContextStrategy {
public:
  void prepareWindowCreationHints() const override;
  bool createContext(void *window) const override;
};

} // namespace Window
