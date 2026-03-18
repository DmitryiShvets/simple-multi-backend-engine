#pragma once

#include <cstdint>
#include <functional>

namespace ssme {

/**
 * @brief Resource ID - lightweight handle for GPU resources
 *
 * Used to reference buffers, textures, pipelines, descriptor sets, etc.
 * Copyable, comparable, but not dereferenceable (no direct pointer semantics).
 *
 * RID with id=0 represents an invalid/null resource.
 */
struct RID {
  uint64_t id = 0;

  /// Constant for invalid/null resource ID
  static const RID INVALID;

  RID() = default;
  explicit constexpr RID(uint64_t in_id) : id(in_id) {}

  /// Check if this RID is valid (non-null)
  [[nodiscard]] constexpr bool isValid() const { return id != 0; }

  /// Check if this RID is null (invalid)
  [[nodiscard]] constexpr bool isNull() const { return id == 0; }

  explicit operator bool() const { return isValid(); }
};

// Definition of static member
inline const RID RID::INVALID{0};

inline constexpr bool operator==(const RID &lhs, const RID &rhs) {
  return lhs.id == rhs.id;
}
inline constexpr bool operator!=(const RID &lhs, const RID &rhs) {
  return lhs.id != rhs.id;
}
inline constexpr bool operator<(const RID &lhs, const RID &rhs) {
  return lhs.id < rhs.id;
}
inline constexpr bool operator<=(const RID &lhs, const RID &rhs) {
  return lhs.id <= rhs.id;
}
inline constexpr bool operator>(const RID &lhs, const RID &rhs) {
  return lhs.id > rhs.id;
}
inline constexpr bool operator>=(const RID &lhs, const RID &rhs) {
  return lhs.id >= rhs.id;
}

} // namespace ssme

namespace std {
template <> struct hash<ssme::RID> {
  std::size_t operator()(const ssme::RID &rid) const {
    return hash<uint64_t>()(rid.id);
  }
};
} // namespace std
