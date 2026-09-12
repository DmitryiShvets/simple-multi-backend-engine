#include "input_system.h"
#include "event_bus.h"
#include "input_events.h"

namespace ssme {

InputSystem::InputSystem(EventBus &bus) : m_bus(bus) {}

InputState &InputSystem::stateFor(std::size_t window_id) {
  InputState &st = m_states[window_id];
  st.window_id = window_id;
  return st;
}

const InputState &InputSystem::getState(std::size_t window_id) const {
  static const InputState empty;
  auto it = m_states.find(window_id);
  return it != m_states.end() ? it->second : empty;
}

bool InputSystem::isKeyDown(std::size_t window_id, Key key) const {
  auto it = m_states.find(window_id);
  if (it == m_states.end()) {
    return false;
  }
  auto key_it = it->second.keys.find(key);
  return key_it != it->second.keys.end() && key_it->second;
}

void InputSystem::onKey(std::size_t window_id, Key key, KeyActionType action,
                        int mods) {
  InputState &st = stateFor(window_id);

  switch (action) {
  case KeyActionType::Press:
    st.keys[key] = true;
    m_bus.publish(KeyPressedEvent(key, mods, window_id, /*repeat=*/false));
    break;
  case KeyActionType::Release:
    st.keys[key] = false;
    m_bus.publish(KeyReleasedEvent(key, mods, window_id));
    break;
  case KeyActionType::Repeat:
    m_bus.publish(KeyPressedEvent(key, mods, window_id, /*repeat=*/true));
    break;
  }
}

void InputSystem::onMouseButton(std::size_t window_id, MouseButton button,
                                KeyActionType action, int mods, double x, double y) {
  InputState &st = stateFor(window_id);
  st.buttons[button] = (action == KeyActionType::Press);
  m_bus.publish(MouseButtonEvent(button, action, mods, x, y, window_id));

}

void InputSystem::onMouseMove(std::size_t window_id, double x, double y) {
  InputState &st = stateFor(window_id);
  st.mouse_dx = x - st.mouse_x;
  st.mouse_dy = y - st.mouse_y;
  st.mouse_x = x;
  st.mouse_y = y;
  m_bus.publish(MouseMovedEvent(x, y, st.mouse_dx, st.mouse_dy, window_id));
}

void InputSystem::onScroll(std::size_t window_id, double xoff, double yoff) {
  InputState &st = stateFor(window_id);
  st.scroll_dx += xoff;
  st.scroll_dy += yoff;
  m_bus.publish(MouseScrolledEvent(xoff, yoff, window_id));
}

void InputSystem::beginFrame() {
  for (auto &[id, st] : m_states) {
    (void)id;
    st.mouse_dx = 0.0;
    st.mouse_dy = 0.0;
    st.scroll_dx = 0.0;
    st.scroll_dy = 0.0;
  }
}

bool InputSystem::isMouseDown(std::size_t window_id, MouseButton button) const {
  auto it = m_states.find(window_id);
  if (it == m_states.end()) {
    return false;
  }
  auto btn_it = it->second.buttons.find(button);
  return btn_it != it->second.buttons.end() && btn_it->second;
}

} // namespace ssme
