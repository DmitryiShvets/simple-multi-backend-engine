#pragma once
#include "buffer.h"
#include "render_device.h"

namespace ssme {
Buffer::Buffer(const std::string &id,
               const std::vector<RenderDevice *> &devices)
    : Resource(id, devices) {}
Buffer::Buffer(const BufferDesc &desc,
               const std::vector<RenderDevice *> &devices)
    : Resource("id", devices) {}

bool doLoad() { return true; }
bool doUnload() { return true; }
} // namespace ssme
