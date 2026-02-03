#pragma once
#include <flecs.h>

namespace Core::Ecs
{
    // Concrete implementation of Query for Flecs
    template <typename... Components>
    class FlecsQueryImpl
    {
    public:
        // Constructor accepts a pointer to the flecs world
        FlecsQueryImpl(flecs::world* world) : m_query(world->query<Components...>()) {}

        // Implementation of the 'each' method.
        // It adapts the flecs::entity to our EntityHandle and forwards all components.
        template<typename Func>
        void each(Func&& func)
        {
            m_query.each([&func](flecs::entity e, Components&... comps) {
                func(e.id(), comps...);
            });
        }

    private:
        flecs::query<Components...> m_query;
    };
}
