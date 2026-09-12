#pragma once

namespace ssme {

// Event categories
enum class EventCategory {
  None = 0,
  Application = 1 << 0,
  Input = 1 << 1,
  Keyboard = 1 << 2,
  Mouse = 1 << 3,
  MouseButton = 1 << 4,
  Window = 1 << 5,
  UI = 1 << 6,
};

// Base event class
class Event {
public:
  virtual ~Event() = default;

  // Get the type of the event
  virtual const char *getType() const = 0;

  // Clone the event (for queued events)
  virtual Event *clone() const = 0;

  // Get the categories this event belongs to
  virtual int getCategoryFlags() const = 0;

  // Check if event is in category
  bool isInCategory(EventCategory category) const {
    return getCategoryFlags() & static_cast<int>(category);
  }
};

// Macro to help define event types with categories
#define DEFINE_EVENT_TYPE(type, categoryFlags)                                 \
  static const char *getStaticType() { return #type; }                         \
  virtual const char *getType() const override { return getStaticType(); }     \
  virtual Event *clone() const override { return new type(*this); }            \
  virtual int getCategoryFlags() const override { return categoryFlags; }

// Event listener interface
class EventListener {
public:
  virtual ~EventListener() = default;
  virtual void onEvent(const Event &event) = 0;
};

// Event dispatcher
class EventDispatcher {
public:
  explicit EventDispatcher(const Event &e) : event(e) {}

  // Dispatch event to handler if types match
  template <typename T, typename F> bool dispatch(const F &handler) {
    if (event.getType() == T::getStaticType()) {
      handler(static_cast<const T &>(event));
      return true;
    }
    return false;
  }

private:
  const Event &event;
};

} // namespace ssme
