#include "world_base.h"
#include "ecs/common_tags.h"
#include "utils/debug_assert.h"
#include "utils/hash_utils.h"
#include <cstddef>
#include <flecs.h>
#include <flecs/addons/cpp/c_types.hpp>
#include <stdexcept>
#include <string>

namespace ssme {
struct WorldBase::Impl {
  flecs::world world;
  std::unordered_map<size_t, flecs::entity> comp_cache;
  std::unordered_map<size_t, flecs::query<>> query_cache;

  flecs::entity get_comp(size_t type_hash, const char *name, size_t s, size_t a,
                         const TypeLifecycle *lc) {
    auto it = comp_cache.find(type_hash);
    if (it != comp_cache.end() && it->second.is_alive())
      return it->second;

    if (it != comp_cache.end())
      comp_cache.erase(it);

    ecs_component_desc_t desc = {};
    desc.entity = 0;
    desc.type.alignment = (ecs_size_t)a;
    desc.type.size = (ecs_size_t)s;
    // 2. If there's a nontrivial lifecycle — attach hooks
    if (lc) {
      ecs_type_hooks_t hooks = {};

      hooks.ctx = (void *)lc; // Pass our static TypeLifecycle

      // Bind our functions to Flecs signatures
      hooks.ctor = (ecs_xtor_t)lc->ctor;
      hooks.dtor = (ecs_xtor_t)lc->dtor;
      hooks.copy = (ecs_copy_t)lc->copy;
      hooks.move = (ecs_move_t)lc->move;
      hooks.copy_ctor = (ecs_copy_t)lc->copy_ctor;
      hooks.move_ctor = (ecs_move_t)lc->move_ctor;

      desc.type.hooks = hooks;
    }

    ecs_entity_t comp_id = ecs_component_init(world.c_ptr(), &desc);
    if (comp_id == 0) {
      throw std::runtime_error("FAILED: register component");
    }

    ecs_set_name(world.c_ptr(), comp_id, name);
    flecs::entity comp(world, comp_id);

    comp_cache[type_hash] = comp;
    return comp;
  }
};

WorldBase::WorldBase() : m_impl(new Impl()) {
  // init system tags
  createEntity(to_string(ECS_TAGS::RENDER_PENDING));
  createEntity(to_string(ECS_TAGS::DESTROYED));
}
WorldBase::~WorldBase() { delete m_impl; }

EntityID WorldBase::createEntity(const std::string &name) {
  return m_impl->world.entity(name.empty() ? nullptr : name.c_str()).id();
}

EntityID WorldBase::getEntity(const std::string &name) const {
  return m_impl->world.lookup(name.c_str());
}

std::string WorldBase::getEntityName(EntityID id) const {
  flecs::entity e = m_impl->world.entity(id);
  std::string name{e.name()};
  return name;
}

bool WorldBase::destroyEntity(EntityID id) {
  auto ent = m_impl->world.entity(id);
  auto deleted = getEntity(to_string(ECS_TAGS::DESTROYED));
  if (ent.is_alive()) {
    ent.add(deleted);
    return true;
  }
  return false;
}

bool WorldBase::destruct(EntityID id) {
  auto ent = m_impl->world.entity(id);
  if (ent.is_alive()) {
    ent.destruct();
    return true;
  }
  return false;
}

bool WorldBase::isEntityAlive(EntityID id) const {
  return m_impl->world.entity(id).is_alive();
}

bool WorldBase::_addComponentRaw(EntityID id, size_t type_hash,
                                 const char *type_name, size_t size,
                                 size_t align, const void *data,
                                 const TypeLifecycle *lc, bool is_move) {
  auto ent = m_impl->world.entity(id);
  if (!ent.is_alive())
    return false;

  // auto comp = m_impl->get_comp(type_hash, type_name, size, align, lc);
  auto world_ptr = m_impl->world.c_ptr();
  // 1. Get/register component ID (with our dtor/move_ctor hooks)
  auto comp_id = m_impl->get_comp(type_hash, type_name, size, align, lc);
  // ent.set_ptr(comp, data);
  // 2. Allocate "raw" memory.
  // The is_new flag will tell us whether to call the constructor.
  bool is_new = false;
  void *ecs_mem = ecs_emplace_id(world_ptr, id, comp_id, size, &is_new);

  if (is_new) {
    // CASE 1: Component didn't exist before.
    // We're creating it for the FIRST time directly in Flecs memory.
    if (lc) {
      if (is_move) {
        // Call C++ Move-Constructor
        lc->emplace_move(ecs_mem, const_cast<void *>(data));
      } else {
        // Call C++ Copy-Constructor
        lc->emplace_copy(ecs_mem, data);
      }
    } else {
      // For POD types, just copy bytes
      memcpy(ecs_mem, data, size);
    }
  } else {
    // CASE 2: Component already exists on entity.
    // Here we should use assignment operator (Copy/Move assignment)
    // or just overwrite memory if that's acceptable.
    if (lc) {
      // Call destructor of old object and create new one in its place
      lc->dtor(ecs_mem, 1, nullptr);
      if (is_move) {
        lc->emplace_move(ecs_mem, const_cast<void *>(data));
      } else {
        lc->emplace_copy(ecs_mem, data);
      }
    } else {
      memcpy(ecs_mem, data, size);
    }
  }

  // 3. Tell Flecs that data changed (so OnSet observers trigger)
  ecs_modified_id(world_ptr, id, comp_id);

  return true;
}

void *WorldBase::_getComponentRaw(EntityID id, size_t type_hash) {
  auto it = m_impl->comp_cache.find(type_hash);
  if (it == m_impl->comp_cache.end()) {
    return nullptr;
  }
  auto ent = m_impl->world.entity(id);
  if (!ent.is_alive() || !ent.has(it->second)) {
    return nullptr;
  }
  return ent.get_mut(it->second); // ✅ get_mut works everywhere
}

bool WorldBase::_hasComponentRaw(EntityID id, size_t type_hash) const {
  auto it = m_impl->comp_cache.find(type_hash);
  if (it == m_impl->comp_cache.end()) {
    return false;
  }
  auto ent = m_impl->world.entity(id);
  return ent.is_alive() && ent.has(it->second);
}

bool WorldBase::_removeComponentRaw(EntityID id, size_t type_hash) {
  auto it = m_impl->comp_cache.find(type_hash);
  if (it == m_impl->comp_cache.end())
    return false;

  auto ent = m_impl->world.entity(id);
  if (!ent.is_alive())
    return false;

  if (!ent.has(it->second)) {
    return false;
  }
  ent.remove(it->second);
  return true;
}

bool WorldBase::_addTagRaw(EntityID id, EntityID tag_id) {
  auto ent = m_impl->world.entity(id);
  if (ent.is_alive()) {
    ent.add(tag_id);
    return true;
  }
  return false;
}

bool WorldBase::_hasTagRaw(EntityID id, EntityID tag_id) const {
  auto ent = m_impl->world.entity(id);
  return ent.is_alive() && ent.has(tag_id);
}

bool WorldBase::_removeTagRaw(EntityID id, EntityID tag_id) {
  auto ent = m_impl->world.entity(id);
  if (ent.is_alive()) {
    ent.remove(tag_id);
    return true;
  }
  return false;
}

void WorldBase::_viewImplRaw(const std::vector<size_t> &type_hashes,
                             const std::vector<EntityID> &tag_ids,
                             RawEachFunc callback) {
  // Query key generation
  size_t query_key = 0;
  for (auto h : type_hashes) {
    hash_combine(query_key, h);
  }
  for (auto t : tag_ids) {
    hash_combine(query_key, t);
  }

  // Query construction
  if (m_impl->query_cache.find(query_key) == m_impl->query_cache.end()) {
    auto builder = m_impl->world.query_builder();
    for (auto h : type_hashes) {
      auto it = m_impl->comp_cache.find(h);
      if (it != m_impl->comp_cache.end()) {
        builder.with(it->second);
      }
    }
    // Add tags as separate filters
    for (auto tag_id : tag_ids) {
      builder.with(m_impl->world.entity(tag_id));
    }
    m_impl->query_cache[query_key] = builder.build();
  }

  auto &q = m_impl->query_cache[query_key];
  q.each([&](flecs::iter &it, size_t row) {
    // it is the iterator of current table
    // row is the index of specific entity inside this table.
    std::vector<void *> ptrs;
    ptrs.reserve(type_hashes.size()); // Allocation optimization

    // each pointer is the address of data for a specific component
    for (size_t i = 0; i < type_hashes.size(); ++i) {
      auto it_comp = m_impl->comp_cache.find(type_hashes[i]);
      if (it_comp == m_impl->comp_cache.end())
        return;

      // 1. Find column index in current iterator by component ID
      int32_t col_idx = it.table().column_index(it_comp->second);
      // int32_t col_idx = it.range().column_index(it_comp->second);
      if (col_idx == -1)
        return; // Component not found in this table

      // 2. Get typed or void* pointer to column start
      void *column_start = it.table().get_column(col_idx);
      // void* column_start = it.range().get_column(col_idx);

      // 3. Calculate address of specific row
      void *ptr = static_cast<char *>(column_start) +
                  (row * it.table().column_size(col_idx));

      ptrs.push_back(ptr);
    }
    callback(it.entity(row).id(), ptrs.data());
  });
}

void WorldBase::_internalObserveAdd(size_t hash, const char *name, size_t s,
                                    size_t a, ObserveCallback cb,
                                    const TypeLifecycle *lc) {
  auto comp = m_impl->get_comp(hash, name, s, a, lc);
  debug_assert(comp.is_alive(), "Cannot registrate component!");
  // Flecs 4.x Observer: triggers on component addition
  m_impl->world.observer()
      .with(comp)
      .event(flecs::OnAdd)
      .each(
          [cb, this](flecs::iter &it, size_t row) { cb(it.entity(row).id()); });
}

void WorldBase::_internalObserveSet(size_t hash, const char *name, size_t s,
                                    size_t a, ObserveCallback cb,
                                    const TypeLifecycle *lc) {
  auto comp = m_impl->get_comp(hash, name, s, a, lc);

  m_impl->world.observer()
      .with(comp)
      .event(flecs::OnSet) // <--- Key difference
      .each([cb](flecs::iter &it, size_t row) { cb(it.entity(row).id()); });
}

void WorldBase::_internalObserveRemove(size_t hash, ObserveCallback cb) {
  auto comp = m_impl->comp_cache[hash];
  debug_assert(comp.is_alive(),
               "Unknown component! Cannot remove unregistred component!");
  m_impl->world.observer()
      .with(comp)
      .event(flecs::OnRemove)
      .each(
          [cb, this](flecs::iter &it, size_t row) { cb(it.entity(row).id()); });
}

void WorldBase::_internalObserveDestroy(ObserveCallback cb) {
  // OnDelete triggers when any entity is deleted
  // auto comp = m_impl->comp_cache[(size_t)ECS_TAGS::DESTROYED];
  // m_impl->world.observer()
  //     .with(comp)
  //     .event(flecs::OnSet)
  //     .each([cb](flecs::iter &it, size_t row) { cb(it.entity(row).id()); });
}
} // namespace ssme
