#pragma once

#include <cstdint>
#include <functional>

namespace Core {

struct RID {
    uint64_t id = 0;
    explicit operator bool() const { return id != 0; }
};

inline bool operator==(const RID& lhs, const RID& rhs) { return lhs.id == rhs.id; }
inline bool operator!=(const RID& lhs, const RID& rhs) { return lhs.id != rhs.id; }
inline bool operator<(const RID& lhs, const RID& rhs) { return lhs.id < rhs.id; }

} // namespace Core

namespace std {
template <> struct hash<Core::RID> {
    std::size_t operator()(const Core::RID& rid) const {
        return hash<uint64_t>()(rid.id);
    }
};
} // namespace std