#pragma once

#include <cstdint>
#include <functional>

namespace Render {
enum class ResourceState { UNDEFINED, RENDER_TARGET, PRESENT_SRC, TRANSFER_DST };
// A lightweight, opaque handle for a rendering resource.
class RID {
public:
    RID() : m_id(0) {}
    explicit RID(uint64_t id) : m_id(id) {}

    bool operator==(const RID& other) const { return m_id == other.m_id; }
    bool operator!=(const RID& other) const { return m_id != other.m_id; }
    bool operator<(const RID& other) const { return m_id < other.m_id; }

    bool isValid() const { return m_id != 0; }
    uint64_t getID() const { return m_id; }

private:
    uint64_t m_id;
};

// Describes the type of a resource behind a RID.
enum class ResourceType {
    Undefined,
    SwapChain,
    Buffer,
    Texture,
    Pipeline,
};

// A placeholder descriptor for creating a swap chain.
// We will add properties to it later (like vsync, format, etc.).
struct SwapChainDesc {};

} // namespace Render

namespace std {
template <>
struct hash<Render::RID> {
    std::size_t operator()(const Render::RID& rid) const {
        return std::hash<uint64_t>()(rid.getID());
    }
};
} // namespace std
