#pragma once

namespace ssme::vulkan {

template <bool THREAD_SAFE> class VulkanGpuStorage;

// Typedefs for convenience
using VulkanGpuStorageMT = VulkanGpuStorage<true>;
using VulkanGpuStorageST = VulkanGpuStorage<false>;

} // namespace ssme::vulkan
