#pragma once

#include "core/input_enums.h"

#include <cstddef>
#include <unordered_map>

namespace ssme {

struct InputState {
  std::size_t window_id = 0;

  // Текущее состояние клавиш (гибрид: события обновляют, системы опрашивают)
  std::unordered_map<Key, bool> keys;
  // Текущее состояние кнопок мыши
  std::unordered_map<MouseButton, bool> buttons;

  // Накопленные за кадр дельты (обнуляются в beginFrame)
  double mouse_x = 0.0;
  double mouse_y = 0.0;
  double mouse_dx = 0.0;
  double mouse_dy = 0.0;
  double scroll_dx = 0.0;
  double scroll_dy = 0.0;
};

} // namespace ssme
