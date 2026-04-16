#include "scene_view_system.h"
#include "ecs/common_tags.h"
#include "ecs/components/material_component.h"
#include "ecs/components/mesh_component.h"
#include "ecs/world.h"
#include "render_item.h"
#include "render_proxy.h"
#include "utils/logger.h"
#include <string>

namespace ssme {

void SceneViewSystem::awake(World &world) {
  // Subscribe to mesh and material appearance
  world.observeSet<MeshComponent>(
      [this](EntityID id, World &w) { build(id, w); });
  world.observeSet<MaterialComponent>(
      [this](EntityID id, World &w) { build(id, w); });
  // If mesh or material was removed — remove render proxy too
  world.observeRemove<MeshComponent>(
      [this](EntityID id, World &w) { rem(id, w); });
  world.observeRemove<MaterialComponent>(
      [this](EntityID id, World &w) { rem(id, w); });
  // Get unique ID for our tag
  m_add_tag = world.getEntity(to_string(ECS_TAGS::RENDER_PENDING)).id();
  m_del_tag = world.getEntity(to_string(ECS_TAGS::DESTROYED)).id();
}

void SceneViewSystem::update(TimeDelta dt, World &world) {
  // Clear views for current frame
  // m_view.opaque_objects.clear();
  auto add = world.view<RenderProxy>().withTag(m_add_tag);
  auto del = world.view<RenderProxy>().withTag(m_del_tag);
  // auto v = world.view<RenderProxy>();

  // Collect entities for separate processing to avoid modifying them during
  // iteration
  std::vector<EntityID> to_process_add;
  std::vector<EntityID> to_process_del;
  add.each([&to_process_add, &world, this](EntityID e, RenderProxy &c) {
    to_process_add.push_back(e);
  });
  del.each([&to_process_del, &world, this](EntityID e, RenderProxy &c) {
    to_process_del.push_back(e);
  });
  // Now safely process
  for (EntityID e : to_process_add) {
    // if (world.hasTag(e, m_ready_tag)) {
    auto proxy = world.getComponent<RenderProxy>(e);
    m_view.opaque_objects.push_back(proxy->render_item);
    world.removeTag(e, m_add_tag);
    // }
  }
  for (EntityID e : to_process_del) {
    // if (world.hasTag(e, m_ready_tag)) {
    std::erase_if(m_view.opaque_objects,
                  [e](RenderItem &item) { return item.ent == e; });
    // }
  }
}

void SceneViewSystem::build(EntityID id, World &w) {
  Entity ent(id, w);

  // We need both components for rendering
  if (ent.has<MeshComponent>() && ent.has<MaterialComponent>()) {

    // If proxy already exists (e.g., material was updated) — can update
    // existing or just recreate RenderProxy component
    const auto &mesh_handle = ent.get<MeshComponent>()->handle;
    const auto &mat_handle = ent.get<MaterialComponent>()->handle;
    auto *mesh_res = mesh_handle.get();
    auto *mat_res = mat_handle.get();

    if (!mesh_res || !mat_res)
      return; // Resources are still loading
    auto *existing_proxy = ent.get<RenderProxy>();

    // CHECK: Do we need to change anything?
    // We rebuild proxy only if resources themselves changed (handles)
    if (existing_proxy) {
      bool mesh_same = existing_proxy->render_item.vertex_buffers[0] ==
                       mesh_res->getVertexBuffer();
      const auto &mat_descriptors = mat_res->getDescriptors();
      bool mat_same = existing_proxy->render_item.descriptor_sets.size() ==
                      mat_descriptors.size();

      for (int i = 0, j = 2; i < mat_descriptors.size(); i++, j++) {
        const auto &id = mat_descriptors[i];
        mat_same =
            mat_same && id == existing_proxy->render_item.descriptor_sets[j];
      }

      if (mesh_same && mat_same) {
        return; // Data is the same, exit
      }
    }

    m_proxy_builder.buildProxy(ent);
    m_proxy_builder.ensureConsistency(ent);
    w.addTag(ent, m_add_tag);
  }
}
void SceneViewSystem::rem(EntityID id, World &w) {}
} // namespace ssme
