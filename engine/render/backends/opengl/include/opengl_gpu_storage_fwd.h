#pragma once

namespace ssme::opengl {

template <bool THREAD_SAFE> class OpenGLGpuStorage;

// Typedefs for convenience
using OpenGLGpuStorageMT = OpenGLGpuStorage<true>;
using OpenGLGpuStorageST = OpenGLGpuStorage<false>;

} // namespace ssme::opengl
