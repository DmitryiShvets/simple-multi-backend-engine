#pragma once
#include <flecs.h>
#include "ecs_types.h"
#include "query.h"
#include "flecs_query.h"

namespace Core::Ecs
{
    // Concrete implementation of a World for Flecs.
    // This class satisfies the EcsWorld concept.
    class FlecsWorldImpl
    {
    public:
        FlecsWorldImpl() = default;

        EntityHandle createEntity(const std::string& name = {})
        {
            return static_cast<EntityHandle>(m_world.entity().id());
        }

        template <typename Component, typename... Args>
        void addComponent(EntityHandle entity, Args&&... args)
        {
            m_world.entity(entity).set<Component>({std::forward<Args>(args)...});
        }

        template <typename Component>
        Component& getComponent(EntityHandle entity)
        {
            return m_world.entity(entity).get_mut<Component>();
        }

        template <typename Component>
        bool hasComponent(EntityHandle entity)
        {
            return m_world.entity(entity).has<Component>();
        }

        template <typename... Components>
        auto createQuery()
        {
            // Return our abstract Query object,
            // which contains the implementation for Flecs inside.
            return Query<FlecsQueryImpl<Components...>>(&m_world);
        }

        // Add a method for direct access to the flecs world, if needed
        // in higher-level logic (e.g., for configuring flecs systems).
        flecs::world& getNativeWorld()
        {
            return m_world;
        }

    private:
        flecs::world m_world;
    };
}
