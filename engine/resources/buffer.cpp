#pragma once
#include "buffer.h"
#include "core/uuid.h"
#include "render_device.h"

namespace ssme {

Buffer::Buffer(const BufferDesc &desc, const VecRefRD &devices)
    : Resource(genUuidV4(), devices) {}

Buffer::~Buffer() {
  unload(); // Ensure proper cleanup when object is destroyed
}

bool Buffer::doLoad() { return true; }
bool Buffer::doUnload() { return true; }
} // namespace ssme
