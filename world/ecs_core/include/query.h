#pragma once
#include "concepts.h"

namespace Core::Ecs
{
    template <typename QueryImpl> requires EcsQuery<QueryImpl>
    class Query
    {
    public:
        // Constructor that forwards arguments to the implementation
        template<typename... Args>
        Query(Args&&... args) : m_impl(std::forward<Args>(args)...) {}

        template<typename Func>
        void each(Func&& func)
        {
            // Statically check that the QueryImpl satisfies the EcsQuery concept
            // static_assert(EcsQuery<QueryImpl>, "QueryImpl must satisfy the EcsQuery concept");
            impl().each(std::forward<Func>(func));
        }

    private:
        QueryImpl& impl() { return m_impl; }
        const QueryImpl& impl() const { return m_impl; }

        QueryImpl m_impl;
    };
}
