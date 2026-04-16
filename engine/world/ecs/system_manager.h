#pragma once
#include "core/time.h"
#include "system.h"
#include <vector>
#include <memory>

namespace ssme {

class SystemManager {
public:
    explicit SystemManager(World& world) : m_world(world) {}

    // Add system: SystemManager will create it and call awake
    template<typename T, typename... Args>
    T* addSystem(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = system.get();

        system->awake(m_world);
        m_systems.push_back(std::move(system));

        return ptr;
    }

    // Main method to call in game loop
    void update(TimeDelta dt) {
        // 1. First run Start for new systems
        for (auto& sys : m_systems) {
            if (sys->active && !sys->m_started) {
                sys->start(m_world);
                sys->m_started = true;
            }
        }

        // 2. Main update
        for (auto& sys : m_systems) {
            if (sys->active) {
                sys->update(dt, m_world);
            }
        }

        // 3. Late update (cameras, UI)
        for (auto& sys : m_systems) {
            if (sys->active) {
                sys->lateUpdate(dt, m_world);
            }
        }
    }

    void shutdown() {
        for (auto& sys : m_systems) {
            sys->onDestroy(m_world);
        }
        m_systems.clear();
    }

private:
    World& m_world;
    std::vector<std::unique_ptr<System>> m_systems;
};

} // namespace ssme
