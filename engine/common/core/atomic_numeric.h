#pragma once

#include <atomic>

namespace ssme {
template <class T> class AtomicNumeric {
  std::atomic<T> value;

public:
  T increment() { return value.fetch_add(1, std::memory_order_acq_rel) + 1; }

  // Returns the original value instead of the new one
  T postincrement() { return value.fetch_add(1, std::memory_order_acq_rel); }

  T decrement() { return value.fetch_sub(1, std::memory_order_acq_rel) - 1; }

  T get() const { return value.load(); }
};
} // namespace ssme
