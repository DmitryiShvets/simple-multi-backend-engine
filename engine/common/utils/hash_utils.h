#pragma once
#include <cstddef>
#include <functional>

namespace ssme {

template<class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<class T, class... Args>
inline void hash_combine(std::size_t& seed, const T& v, Args... args) {
    hash_combine(seed, v);
    hash_combine(seed, args...);
}

} // namespace ssme
