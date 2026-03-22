#include "rid_allocator.h"
#include "core/rid.h"
#include <limits>
#include <vector>

namespace ssme {

RIDAllocator::RIDAllocator(uint64_t range_start, uint64_t range_end)
    : m_range_start(range_start),
      m_range_end(range_end == 0 ? std::numeric_limits<uint64_t>::max()
                                 : range_end),
      m_next_id(0) {}

RID RIDAllocator::allocate() {
  // Try to reuse a freed RID first (fast path with lock)
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_free_list.empty()) {
    // ✅ Reuse freed RID (LIFO order for cache locality)
    uint64_t id = m_free_list.back();
    m_free_list.pop_back();
    return RID{id};
  }

  // ✅ No freed RIDs available, allocate new sequential ID
  uint64_t next = m_next_id.fetch_add(1, std::memory_order_relaxed);
  uint64_t id = m_range_start + next + 1; // +1 because RID 0 = invalid

  // Check range limit
  if (id >= m_range_end) {
    return RID::INVALID; // Range exhausted
  }

  return RID{id};
}

std::vector<RID> RIDAllocator::allocate(uint32_t count) {
  if (count <= 0)
    return {};
  std::vector<RID> result;
  for (uint32_t i = 0; i < count; i++) {
    result.push_back(allocate());
  }
  return result;
}

void RIDAllocator::free(RID rid) {
  if (!rid.isValid()) {
    return; // Ignore invalid RIDs
  }

  // Optional: verify RID is within our range
  if (rid.id < m_range_start || rid.id >= m_range_end) {
    return; // Wrong range, ignore
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  m_free_list.push_back(rid.id);
}

void RIDAllocator::free(std::vector<RID> rids) {
  if (rids.size() <= 0)
    return;
  for (auto &rid : rids) {
    free(rid);
  }
}

bool RIDAllocator::isValid(RID rid) const {
  if (!rid.isValid()) {
    return false;
  }

#ifndef NDEBUG
  // Debug build: check if RID is in free-list (freed)
  std::lock_guard<std::mutex> lock(m_mutex);
  for (uint64_t freed_id : m_free_list) {
    if (freed_id == rid.id) {
      return false; // This RID was freed
    }
  }
#endif

  // Release build: only check if RID is non-zero
  return true;
}

uint64_t RIDAllocator::getLiveCount() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  uint64_t total_allocated = m_next_id.load(std::memory_order_relaxed);
  return total_allocated - m_free_list.size();
}

uint64_t RIDAllocator::getTotalAllocated() const {
  return m_next_id.load(std::memory_order_relaxed);
}

size_t RIDAllocator::getFreeListSize() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_free_list.size();
}

void RIDAllocator::reserveFreeList(size_t expected_max) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_free_list.reserve(expected_max);
}

void RIDAllocator::reset() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_next_id.store(0, std::memory_order_relaxed);
  m_free_list.clear();
}
} // namespace ssme
