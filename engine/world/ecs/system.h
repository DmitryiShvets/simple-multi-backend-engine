#pragma once
#include "core/time.h"
#include "world.h"

namespace ssme {

class System {
public:
  virtual ~System() = default;

  // --- Lifecycle hooks (Mono-style) ---
  virtual void awake(World &world) {}
  virtual void start(World &world) {}
  virtual void update(TimeDelta dt, World &world) = 0;
  virtual void lateUpdate(TimeDelta dt, World &world) {}
  virtual void onDestroy(World &world) {}

  // --- Reactive hooks (ECS-style) ---
  // Called automatically if monitor<T>() methods are called in awake
  virtual void onEntityAdded(Entity entity) {}
  virtual void onEntityRemoved(Entity entity) {}

  // --- Management ---
  bool active = true;
  virtual const char *getName() const = 0;

protected:
  // Register interest in component T (for reactive hooks)
  template <typename T> void monitor(World &world) {
    world.observeAdd<T>([this](EntityID id, World &w) {
      if (this->active)
        this->onEntityAdded(Entity(id, w));
    });
    world.observeRemove<T>([this](EntityID id, World &w) {
      if (this->active)
        this->onEntityRemoved(Entity(id, w));
    });
  }

private:
  bool m_started = false;
  friend class SystemManager;
};

} // namespace ssme
