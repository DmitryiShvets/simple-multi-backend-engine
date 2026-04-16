#include "entity_base.h"
#include "world_base.h" // Full World definition is already here

namespace ssme {

    std::string EntityBase::getName() const {
        return m_world.getEntityName(m_id);
    }


void *EntityBase::getComponentRaw(size_t type_hash) const {
  return m_world._getComponentRaw(m_id, type_hash);
}

bool EntityBase::hasComponentRaw(size_t type_hash) const {
  return m_world._hasComponentRaw(m_id, type_hash);
}

bool EntityBase::addComponentRaw(size_t type_hash, const char *type_name,
                                 size_t size, size_t align, const void *data,
                                 const TypeLifecycle *lc, bool is_move) {
  return m_world._addComponentRaw(m_id, type_hash, type_name, size, align,
                                   data, lc, is_move);
}

bool EntityBase::removeComponentRaw(size_t type_hash) {
  return m_world._removeComponentRaw(m_id, type_hash);
}

} // namespace ssme
