#pragma once
#include <concepts>
#include "ecs_types.h"

namespace Core::Ecs
{
    // Forward declaration
    template<typename...> class FlecsQueryImpl;

    // Concept for Query objects
    template <typename QueryType>
    concept EcsQuery = requires(QueryType query) {
        // Requires a callable 'each' method.
        // The exact signature is determined by the implementation.
        { query.each([](auto&&...){}) };
    };

    // Concept for World objects
    template <typename WorldType>
    concept EcsWorld = requires(WorldType world) {
        { world.createEntity() } -> std::same_as<EntityHandle>;

        // Checks for the presence of a factory method to create queries
        // Note: We can't check the templated createQuery<...> here,
        // so we check its non-template variant.
        { world.template createQuery<>() } -> EcsQuery;
    };
}
