#include "event_bus.h"

#include <algorithm>

namespace ssme {

void EventBus::subscribe(EventListener *listener, int categoryFilter,
                         int priority) {
  listeners.push_back({listener, categoryFilter, priority});
  // Sort listeners by priority (higher priority first)
  std::sort(listeners.begin(), listeners.end(),
            [](const ListenerInfo &a, const ListenerInfo &b) {
              return a.priority > b.priority;
            });
}
void EventBus::unsubscribe(EventListener *listener) {
  auto it = std::find_if(listeners.begin(), listeners.end(),
                         [listener](const ListenerInfo &info) {
                           return info.listener == listener;
                         });
  if (it != listeners.end()) {
    listeners.erase(it);
  }
}

void EventBus::publish(const Event &event) {
  if (m_is_immediate_mode) {
    // Dispatch event immediately
    for (const auto &info : listeners) {
      if (info.category == -1 || (event.getCategoryFlags() & info.category)) {
        info.listener->onEvent(event);
      }
    }
  } else {
    // Queue event for later processing
    std::lock_guard<std::mutex> lock(m_event_queue_mutex);
    m_event_queue.push(std::unique_ptr<Event>(event.clone()));
  }
}

void EventBus::processEvents() {
  if (m_is_immediate_mode)
    return;

  std::queue<std::unique_ptr<Event>> currentEvents;
  {
    std::lock_guard<std::mutex> lock(m_event_queue_mutex);
    std::swap(currentEvents, m_event_queue);
  }

  while (!currentEvents.empty()) {
    auto &event = *currentEvents.front();
    for (const auto &info : listeners) {
      if (info.category == -1 || (event.getCategoryFlags() & info.category)) {
        info.listener->onEvent(event);
      }
    }
    currentEvents.pop();
  }
}
} // namespace ssme
