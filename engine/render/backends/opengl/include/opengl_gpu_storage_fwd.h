#pragma once

namespace ssme::opengl {

template <bool THREAD_SAFE> class OpenGLGpuStorage;

// Typedefs для удобства
using OpenGLGpuStorageMT = OpenGLGpuStorage<true>;
using OpenGLGpuStorageST = OpenGLGpuStorage<false>;

} // namespace ssme::opengl
