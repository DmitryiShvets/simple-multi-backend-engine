#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace ssme {

class WorldBase; // forward declaration
struct TypeLifecycle;

using EntityID = uint64_t;

class EntityBase {
public:
  EntityBase() = delete;
  EntityBase(uint64_t id, WorldBase &world) : m_id(id), m_world(world) {}

  std::string getName() const;

  bool addComponentRaw(size_t type_hash, const char *type_name,
                       size_t size, size_t align, const void *data,
                       const TypeLifecycle *lc, bool is_move);
  void *getComponentRaw(size_t type_hash) const;
  bool hasComponentRaw(size_t type_hash) const;
  void addComponentRaw(size_t type_hash, const char *type_name, size_t size,
                       size_t align, const void *data);
  bool removeComponentRaw(size_t type_hash);

protected:
  WorldBase &m_world;
  EntityID m_id = 0;
};

} // namespace ssme
