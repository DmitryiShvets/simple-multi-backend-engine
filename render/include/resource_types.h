#pragma once
#include "rid.h"

namespace Render {

// --- Core Types ---

using RID = Core::RID;

// --- Enums ---

enum class ResourceType {
  UNDEFINED,
  SWAP_CHAIN,
  TEXTURE,
  IMAGE,
  IMAGE_VIEW,
  BUFFER,
  PIPELINE,
  DESCRIPTOR_SET_LAYOUT,
  PIPELINE_LAYOUT,
};

enum class ResourceState { UNDEFINED, TRANSFER_DST, PRESENT_SRC };

} // namespace Render
