#pragma once

#include "event.h"

#include <memory>
#include <mutex>
#include <queue>
#include <vector>

namespace ssme {
// Event bus
class EventBus {

public:

  void subscribe(EventListener *listener, int categoryFilter = -1,
                   int priority = 0);

  void unsubscribe(EventListener *listener);

  void publish(const Event &event);

  void processEvents();

  void setImmediateMode(bool immediate) { m_is_immediate_mode = immediate; }

private:
  struct ListenerInfo {
    EventListener *listener;
    int category;
    int priority;
  };
  std::vector<ListenerInfo> listeners;
  std::queue<std::unique_ptr<Event>> m_event_queue;
  std::mutex m_event_queue_mutex;
  bool m_is_immediate_mode = true;
};
} // namespace ssme
