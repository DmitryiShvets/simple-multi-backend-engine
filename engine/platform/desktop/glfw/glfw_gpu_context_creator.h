#pragma once

#include "gpu_context_strategy.h"

namespace ssme {

/**
 * @brief OpenGL GPU context creation strategy.
 */
class OpenGLGpuContextCreator : public GpuContextStrategy {
public:
    void prepareWindowCreationHints() const override;
    bool createContext(void* window) const override;
    GpuBackend getGpuBackend() const override;
};

/**
 * @brief Vulkan GPU context creation strategy.
 */
class VulkanGpuContextCreator : public GpuContextStrategy {
public:
    void prepareWindowCreationHints() const override;
    bool createContext(void* window) const override;
    GpuBackend getGpuBackend() const override;
};

/**
 * @brief DirectX 12 GPU context creation strategy.
 */
class Dx12GpuContextCreator : public GpuContextStrategy {
public:
    void prepareWindowCreationHints() const override;
    bool createContext(void* window) const override;
    GpuBackend getGpuBackend() const override;
};

} // namespace ssme
