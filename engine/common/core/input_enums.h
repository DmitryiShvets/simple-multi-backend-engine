#pragma once

namespace ssme {

/**
 * @brief Keyboard key codes.
 */
enum class Key {
  None = 0,
  Escape,
  Enter,
  Tab,
  Space,
  Backspace,
  Delete,
  Num0,
  Num1,
  Num2,
  Num3,
  Num4,
  Num5,
  Num6,
  Num7,
  Num8,
  Num9,
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  ArrowLeft,
  ArrowRight,
  ArrowUp,
  ArrowDown,
  LeftShift,
  RightShift,
  LeftControl,
  RightControl,
  LeftAlt,
  RightAlt
};

/**
 * @brief Mouse button codes.
 */
enum class MouseButton { Left, Right, Middle };

/**
 * @brief Button/key actions.
 */
enum class KeyActionType { Press, Release, Repeat };

enum ModFlag : int {
  ModShift = 1 << 0,
  ModCtrl = 1 << 1,
  ModAlt = 1 << 2,
  ModSuper = 1 << 3,
};

} // namespace ssme
