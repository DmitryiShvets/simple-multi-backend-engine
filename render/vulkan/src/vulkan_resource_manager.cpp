#include "vulkan_resource_manager.h"
#include "vulkan_swap_chain.h"

namespace Render::Vulkan {

VulkanResourceManager::VulkanResourceManager() {
  // Initialize with 1, as 0 is an invalid RID
  m_next_rid.store(1);
}

VulkanResourceManager::~VulkanResourceManager() {
  // The unique_ptrs in the ResourceOwners will handle cleanup automatically.
}

void VulkanResourceManager::free(RID rid) {
  if (!rid.isValid()) {
    return;
  }

  auto it = m_rid_type_map.find(rid);
  if (it == m_rid_type_map.end()) {
    // Optional: log an error about trying to free an unknown resource
    return;
  }

  // Depending on the type, call remove() on the correct owner.
  // The unique_ptr returned by remove() will go out of scope and delete the
  // object.
  switch (it->second) {
  case ResourceType::SwapChain:
    m_swap_chain_owner.remove(rid);
    break;
  case ResourceType::Buffer:
    // m_buffer_owner.remove(rid);
    break;
  case ResourceType::Texture:
    // m_texture_owner.remove(rid);
    break;
  case ResourceType::Pipeline:
    // m_pipeline_owner.remove(rid);
    break;
  default:
    // Optional: log an error for an unhandled resource type
    break;
  }

  m_rid_type_map.erase(it);
}

} // namespace Render::Vulkan
