#pragma once

#include <cstddef>

namespace ssme {

enum class ECS_TAGS : size_t {
  DESTROYED = 0,
  RENDER_READY,
  RENDER_PENDING,
};

enum class Color { Red, Green, Blue };

inline const char *to_string(ECS_TAGS t) {
  switch (t) {
  case ECS_TAGS::DESTROYED:
    return "Red";
  case ECS_TAGS::RENDER_READY:
    return "Green";
  case ECS_TAGS::RENDER_PENDING:
    return "Blue";
  default:
    return "Unknown";
  }
}
} // namespace ssme
