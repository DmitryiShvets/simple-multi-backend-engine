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
};

/**
 * @brief Vulkan GPU context creation strategy.
 */
class VulkanGpuContextCreator : public GpuContextStrategy {
public:
    void prepareWindowCreationHints() const override;
    bool createContext(void* window) const override;
};

} // namespace ssme
