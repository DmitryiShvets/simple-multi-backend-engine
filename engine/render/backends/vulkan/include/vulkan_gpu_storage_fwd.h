#pragma once

namespace ssme::vulkan {

template <bool THREAD_SAFE> class VulkanGpuStorage;

// Typedefs для удобства
using VulkanGpuStorageMT = VulkanGpuStorage<true>;
using VulkanGpuStorageST = VulkanGpuStorage<false>;

} // namespace ssme::vulkan
