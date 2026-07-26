#pragma once

namespace ssme::d3d12 {

template <bool THREAD_SAFE> class Dx12GpuStorage;

// Typedefs for convenience
using Dx12GpuStorageMT = Dx12GpuStorage<true>;
using Dx12GpuStorageST = Dx12GpuStorage<false>;

} // namespace ssme::d3d12
