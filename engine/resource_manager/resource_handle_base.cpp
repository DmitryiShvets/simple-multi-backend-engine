#include "resource_handle_base.h"
#include "resource_manager.h"

namespace ssme {

constexpr bool DEBUG = false;

ResourceHandleBase::ResourceHandleBase(const std::string &uuid, uint32_t index,
                                       uint32_t gen, ResourceManager *mgr)
    : m_uuid(uuid), m_slot_index(index), m_expected_gen(gen),
      m_resource_manager(mgr) {}

ResourceHandleBase::ResourceHandleBase(const ResourceHandleBase &other)
    : m_uuid(other.m_uuid), m_slot_index(other.m_slot_index),
      m_expected_gen(other.m_expected_gen),
      m_resource_manager(other.m_resource_manager) {

  // check if handle was constructed by default constructor
  // in this case it is not yet initialized
  if (m_resource_manager && !m_uuid.empty()) {
#ifdef DEBUG
    printf("[HANDLE] CCTOR: %s (ptr: %p, other: %p)\n", m_uuid.c_str(), this,
           &other);
#endif
    if (!m_resource_manager->increment_ref(m_slot_index, m_expected_gen)) {
      throw std::runtime_error(
          "Corrupted Resource Manager! Resource is not found in handle table.");
    }
  }
}

ResourceHandleBase &
ResourceHandleBase::operator=(const ResourceHandleBase &other) {
  // prevent self copy
  if (this != &other) {
    // check if handle was constructed by default constructor
    // in this case it is not yet initialized
    if (m_resource_manager && !m_uuid.empty()) {
      m_resource_manager->release(m_slot_index, m_expected_gen);
    }
    // Copy and increment refcount
    m_uuid = other.m_uuid;
    m_slot_index = other.m_slot_index;
    m_expected_gen = other.m_expected_gen;
    m_resource_manager = other.m_resource_manager;

    // check if handle was constructed by default constructor
    // in this case it is not yet initialized
    if (m_resource_manager && !m_uuid.empty()) {
#ifdef DEBUG
      printf("[HANDLE] COPY: %s (from %p to %p)\n", m_uuid.c_str(), &other,
             this);
#endif
      if (!m_resource_manager->increment_ref(m_slot_index, m_expected_gen)) {
        throw std::runtime_error("Corrupted Resource Manager! Resource is not "
                                 "found in handle table.");
      }
    }
  }
  return *this;
}

ResourceHandleBase::ResourceHandleBase(ResourceHandleBase &&other) noexcept
    : m_uuid(std::move(other.m_uuid)), m_slot_index(other.m_slot_index),
      m_expected_gen(other.m_expected_gen),
      m_resource_manager(other.m_resource_manager) {
  other.m_uuid.clear();
  other.m_slot_index = INVALID_SLOT;  // Invalidate slot
  other.m_expected_gen = INVALID_GEN; // Invalidate gemeration
  other.m_resource_manager = nullptr; // Invalidate source
#ifdef DEBUG
  printf("[HANDLE] MCTOR: %s (from %p to %p)\n", m_uuid.c_str(), &other, this);
#endif
}

ResourceHandleBase &
ResourceHandleBase::operator=(ResourceHandleBase &&other) noexcept {
  // prevent self assign
  if (this != &other) {
    // check if handle was constructed by default constructor
    // in this case it is not yet initialized
    if (m_resource_manager && !m_uuid.empty()) {
      m_resource_manager->release(m_slot_index, m_expected_gen);
    }
    // Move source
    m_uuid = std::move(other.m_uuid);
    m_slot_index = std::move(other.m_slot_index);
    m_expected_gen = std::move(other.m_expected_gen);
    m_resource_manager = std::move(other.m_resource_manager);
    // Invalidate source
    other.m_uuid.clear();
    other.m_slot_index = INVALID_SLOT;  // Invalidate slot
    other.m_expected_gen = INVALID_GEN; // Invalidate gemeration
    other.m_resource_manager = nullptr; // Invalidate source
#ifdef DEBUG
    printf("[HANDLE] MOVE: %s (from %p to %p)\n", m_uuid.c_str(), &other, this);
#endif
  }
  return *this;
}

ResourceHandleBase::~ResourceHandleBase() {
  if (m_resource_manager) {
    // TODO: check that manager is still alive. Handles must be
    // destroyed BEFORE this!
    // Add something like assert(!m_resource_manager->isDestroyed());
    // or need to use weak_ptr<ResourceManager>::lock()
#ifdef DEBUG
    printf("[HANDLE] DTOR: %s (ptr: %p)\n", m_uuid.c_str(), this);
#endif
    m_resource_manager->release(m_slot_index, m_expected_gen);
    m_slot_index = INVALID_SLOT;
    m_expected_gen = INVALID_GEN;
  }
}

Resource *ResourceHandleBase::fetch() const {
  if (!m_resource_manager || m_slot_index == INVALID_SLOT)
    return nullptr;
  auto resource = m_resource_manager->access(m_slot_index, m_expected_gen);
  debug_assert(resource != nullptr,
               "Corrupted Resource Manager! Resource is not "
               "found in handle table.");
  return resource;
}

bool ResourceHandleBase::isValid() const { return fetch() != nullptr; }

} // namespace ssme
