#pragma once
#include "concepts.h"
#include "ecs/components/transform_component.h"
#include "ecs/components/render_component.h"
#include "../../render/include/i_renderer.h"
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace Core::Ecs::System {

    template <typename WorldType> requires EcsWorld<WorldType>
    class RenderSystem {
    public:
        RenderSystem(WorldType& world, Render::IRenderer& renderer)
            : m_world(world),
              m_renderer(renderer),
              m_query(world.template createQuery<const Component::Transform, const Component::Renderable>())
        {
        }

        void update(float /*deltaTime*/) {
            collectRenderData();

            if (m_sortByMaterial && !m_renderables.empty()) {
                sortRenderData();
            }

            if (m_batchingEnabled && !m_renderables.empty()) {
                batchRenderData();
            }

            executeRendering();

            m_renderables.clear();
            m_batches.clear();
        }

        void enableBatching(bool enable) { m_batchingEnabled = enable; }
        void enableSorting(bool enable) { m_sortByMaterial = enable; }

    private:
        WorldType& m_world;
        Render::IRenderer& m_renderer;

        decltype(m_world.template createQuery<const Component::Transform, const Component::Renderable>()) m_query;

        bool m_batchingEnabled = true;
        bool m_sortByMaterial = true;

        std::vector<Component::Renderable> m_renderables;
        std::unordered_map<std::string, std::vector<const Component::Renderable*>> m_batches;

        void collectRenderData() {
            // Now we use our powerful query to get components directly
            m_query.each([this](EntityHandle /*entity*/, const Component::Transform& /*transform*/, const Component::Renderable& renderable) {
                if (renderable.visible) {
                    // Copy data for rendering.
                    // In the future, more complex data conversion logic could go here.
                    m_renderables.push_back(renderable);
                }
            });
        }

        void sortRenderData() {
            std::sort(m_renderables.begin(), m_renderables.end(),
                [](const Component::Renderable& a, const Component::Renderable& b) {
                    return a.shader_id < b.shader_id;
                });
        }

        void batchRenderData() {
            for (const auto& renderable : m_renderables) {
                m_batches[renderable.shader_id].push_back(&renderable);
            }
        }

        void executeRendering() {
            if (m_batchingEnabled && !m_batches.empty()) {
                for (const auto& [shader_id, batch] : m_batches) {
                    for (const auto* renderable : batch) {
                        m_renderer.drawBundle(shader_id, const_cast<void*>(static_cast<const void*>(renderable)));
                    }
                }
            } else if (!m_renderables.empty()) {
                // Non-batched rendering logic would go here
            }
        }
    };

} // namespace Core::Ecs::System
