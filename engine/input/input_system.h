#pragma once

#include "input_state.h"
#include "core/input_enums.h"

#include <cstddef>
#include <unordered_map>

namespace ssme {

class EventBus;

class InputSystem {
public:
  explicit InputSystem(EventBus &bus);

  // === Platform-facing handlers (регистрируются как колбэки платформы) ===
  void onKey(std::size_t window_id, Key key, KeyActionType action, int mods);
  void onMouseButton(std::size_t window_id, MouseButton button, KeyActionType action,
                     int mods, double x, double y);
  void onMouseMove(std::size_t window_id, double x, double y);
  void onScroll(std::size_t window_id, double xoff, double yoff);

  // Обнуляет накопленные дельты в конце кадра
  void beginFrame();

  bool isKeyDown(std::size_t window_id, Key key) const;
  const InputState &getState(std::size_t window_id) const;

  bool isMouseDown(std::size_t window_id, MouseButton button) const;

private:
  InputState &stateFor(std::size_t window_id);

  EventBus &m_bus;
  std::unordered_map<std::size_t, InputState> m_states;
};

} // namespace ssme
