#include "resource_handle_base.h"
#include "resource_manager.h"

namespace ssme {

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
  }
  return *this;
}

ResourceHandleBase::~ResourceHandleBase() {
  if (m_resource_manager) {
    // TODO:  проверить что менеджер ещё жив. handles сами должны быть
    // уничтожены ДО этого!
    //  Добавить что-то типо assert(!m_resource_manager->isDestroyed());
    // или нужно использовть week_ptr<RecourceManager>::lock()
    m_resource_manager->release(m_slot_index, m_expected_gen);
    m_slot_index = INVALID_SLOT;
    m_expected_gen = INVALID_GEN;
  }
}

// новый подход с поколением и кешированием указателя при первом look up
// template <typename T> T *ResourceHandle<T>::get() const {
//   // check if handle was constructed by default constructor
//   // in this case it is not yet initialized
//   if (m_resource_manager && !m_uuid.empty()) {
//     auto resource = m_resource_manager->access(m_slot_index, m_expected_gen);
//     debug_assert(resource != nullptr,
//                  "Corrupted Resource Manager! Resource is not "
//                  "found in handle table.");
//     return static_cast<T *>(resource);
//   } else {
//     return nullptr;
//   }
// }

// template <typename T> bool ResourceHandle<T>::isValid() const {
//   if (!m_resource_manager || m_uuid.empty())
//     return false;
//   return m_resource_manager->has<T>(m_slot_index, m_expected_gen);
// }

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
