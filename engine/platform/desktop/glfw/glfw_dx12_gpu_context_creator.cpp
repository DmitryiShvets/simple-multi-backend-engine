#include "core/gpu_types.h"
#include "glfw_gpu_context_creator.h"
#include "utils/logger.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace ssme {

GpuBackend Dx12GpuContextCreator::getGpuBackend() const {
  return GpuBackend::DirectX12;
}

void Dx12GpuContextCreator::prepareWindowCreationHints() const {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

bool Dx12GpuContextCreator::createContext(void *window) const {
  Logger::info_log("Initialized DirectX version 12");
  return true;
}

} // namespace ssme
