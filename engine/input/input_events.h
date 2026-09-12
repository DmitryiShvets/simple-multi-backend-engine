#pragma once

#include "core/input_enums.h"
#include "event.h"

#include <cstddef>

namespace ssme {

class KeyPressedEvent : public Event {
public:
  KeyPressedEvent(Key key, int mods, std::size_t window_id, bool repeat = false)
      : m_key(key), m_mods(mods), m_window_id(window_id), m_repeat(repeat) {}

  Key getKey() const { return m_key; }
  int getMods() const { return m_mods; }
  std::size_t getWindowId() const { return m_window_id; }
  bool isRepeat() const { return m_repeat; }

  DEFINE_EVENT_TYPE(KeyPressedEvent,
                    static_cast<int>(EventCategory::Input) |
                        static_cast<int>(EventCategory::Keyboard));

private:
  Key m_key;
  int m_mods;
  std::size_t m_window_id;
  bool m_repeat;
};

class KeyReleasedEvent : public Event {
public:
  KeyReleasedEvent(Key key, int mods, std::size_t window_id)
      : m_key(key), m_mods(mods), m_window_id(window_id) {}

  Key getKey() const { return m_key; }
  int getMods() const { return m_mods; }
  std::size_t getWindowId() const { return m_window_id; }

  DEFINE_EVENT_TYPE(KeyReleasedEvent,
                    static_cast<int>(EventCategory::Input) |
                        static_cast<int>(EventCategory::Keyboard));

private:
  Key m_key;
  int m_mods;
  std::size_t m_window_id;
  bool m_repeat;
};

class MouseMovedEvent : public Event {
public:
  MouseMovedEvent(double x, double y, double dx, double dy,
                  std::size_t window_id)
      : m_x(x), m_y(y), m_dx(dx), m_dy(dy), m_window_id(window_id) {}

  double getX() const { return m_x; }
  double getY() const { return m_y; }
  double getDX() const { return m_dx; }
  double getDY() const { return m_dy; }
  std::size_t getWindowId() const { return m_window_id; }

  DEFINE_EVENT_TYPE(MouseMovedEvent,
                    static_cast<int>(EventCategory::Input) |
                        static_cast<int>(EventCategory::Mouse));

private:
  double m_x;
  double m_y;
  double m_dx;
  double m_dy;
  std::size_t m_window_id;
};

class MouseScrolledEvent : public Event {
public:
  MouseScrolledEvent(double xoff, double yoff, std::size_t window_id)
      : m_xoff(xoff), m_yoff(yoff), m_window_id(window_id) {}

  double getXOffset() const { return m_xoff; }
  double getYOffset() const { return m_yoff; }
  std::size_t getWindowId() const { return m_window_id; }

  DEFINE_EVENT_TYPE(MouseScrolledEvent,
                    static_cast<int>(EventCategory::Input) |
                        static_cast<int>(EventCategory::Mouse));
private:
    double m_xoff;
    double m_yoff;
    std::size_t m_window_id;
};

class MouseButtonEvent : public Event {
public:
  MouseButtonEvent(MouseButton button, KeyActionType action, int mods, double x,
                   double y, std::size_t window_id)
      : m_button(button), m_action(action), m_mods(mods), m_x(x), m_y(y),
        m_window_id(window_id) {}

  MouseButton getButton() const { return m_button; }
  KeyActionType getAction() const { return m_action; }
  int getMods() const { return m_mods; }
  double getX() const { return m_x; }
  double getY() const { return m_y; }
  std::size_t getWindowId() const { return m_window_id; }

  DEFINE_EVENT_TYPE(MouseButtonEvent,
                    static_cast<int>(EventCategory::Input) |
                        static_cast<int>(EventCategory::MouseButton));
private:
  MouseButton m_button;
  KeyActionType m_action;
  int m_mods;
  double m_x;
  double m_y;
  std::size_t m_window_id;
};

} // namespace ssme
