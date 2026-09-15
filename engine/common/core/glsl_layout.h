#pragma once

#include <cstdint>

namespace ssme {

// Shared GLSL layout constants. Used by BOTH the GLSL patcher inside the
// Slang compiler and the desktop-GL backend so the flattened descriptor
// bindings and the reserved push-constant UBO binding always match.
inline constexpr uint32_t kOpenglSetStride = 16;   // binding + set * stride
inline constexpr uint32_t kOpenglPushBinding = 31; // reserved push-constant UBO

} // namespace ssme
