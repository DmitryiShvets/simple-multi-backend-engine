#pragma once

#include "rid.h"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

namespace ssme {

/**
 * @brief RID allocator with free-list reuse
 *
 * Generates unique RIDs for GPU resources and reuses freed RIDs
 * to prevent ID exhaustion during long-running applications.
 *
 * Thread-safe: uses mutex for free-list management.
 *
 * @note RID 0 is reserved for invalid/null resources
 * @note Uses LIFO (Last-In-First-Out) free-list for better cache locality
 */
class RIDAllocator {
public:
  RIDAllocator() = default;
  ~RIDAllocator() = default;

  // Non-copyable, non-movable (singleton-like semantics)
  RIDAllocator(const RIDAllocator &) = delete;
  RIDAllocator &operator=(const RIDAllocator &) = delete;
  RIDAllocator(RIDAllocator &&) = delete;
  RIDAllocator &operator=(RIDAllocator &&) = delete;

  /**
   * @brief Allocate a new unique RID
   *
   * Reuses freed RIDs from the free-list if available.
   * Otherwise allocates a new sequential ID.
   *
   * @return New RID (never returns RID 0)
   */
  RID allocate();

  /**
   * @brief Free a previously allocated RID (returns it to free-list)
   *
   * The freed RID can be reused by subsequent allocate() calls.
   *
   * @param rid RID to free (must be valid and previously allocated)
   * @warning Do not free the same RID twice!
   * @warning Do not use RID after freeing (undefined behavior)
   */
  void free(RID rid);

  /**
   * @brief Check if a RID is currently allocated (not freed)
   *
   * @note This is a debug-only check. In release builds,
   *       only validates that RID is non-zero.
   *
   * @param rid RID to check
   * @return true if RID is valid and not freed
   */
  bool isValid(RID rid) const;
  /**
   * @brief Get the number of currently live (allocated but not freed) RIDs
   * @return Number of live allocations
   */
  uint64_t getLiveCount() const;

  /**
   * @brief Get the total number of RIDs ever allocated (including freed)
   * @return Total allocations since creation
   */
  uint64_t getTotalAllocated() const;

  /**
   * @brief Get the number of RIDs in the free-list (available for reuse)
   * @return Number of reusable RIDs
   */
  size_t getFreeListSize() const;

  /**
   * @brief Reserve capacity in the free-list (optimization)
   *
   * Pre-allocates memory for the free-list to avoid reallocations.
   *
   * @param expected_max Expected maximum number of freed RIDs
   */
  void reserveFreeList(size_t expected_max);

  /**
   * @brief Reset the allocator (for testing or shutdown only!)
   *
   * @warning DO NOT call this while any RIDs are still in use!
   * @warning This will invalidate all previously allocated RIDs!
   */
  void reset();

private:
  /// Next ID to allocate when free-list is empty (atomic for thread safety)
  std::atomic<uint64_t> m_next_id{0};

  /// Stack of freed RIDs available for reuse (LIFO order)
  std::vector<uint64_t> m_free_list;

  /// Mutex to protect free-list (allocate/free are thread-safe)
  mutable std::mutex m_mutex;
};

} // namespace ssme
