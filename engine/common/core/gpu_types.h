#pragma once

#include <cstddef>

namespace ssme {

/**
 * @brief Type render backend
 */
enum class GpuBackend : size_t {
  OpenGL = 0,
  Vulkan = 1,
  DirectX12 = 2,
  Count
};

constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;

} // namespace ssme
