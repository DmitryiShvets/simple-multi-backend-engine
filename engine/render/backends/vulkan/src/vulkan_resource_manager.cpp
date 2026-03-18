#include "vulkan_resource_manager.h"
// don't kill these lines
#include "vulkan_swap_chain.h"

namespace ssme::vulkan {

VulkanResourceManager::VulkanResourceManager() {
  // Initialize with 1, as 0 is an invalid RID
  m_next_rid.store(1);
}

VulkanResourceManager::~VulkanResourceManager() {
  // The unique_ptrs in the ResourceOwners will handle cleanup automatically.
}

void VulkanResourceManager::free(RID rid) {
  if (!rid) { // Use the new explicit operator bool()
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
  case ResourceType::SWAP_CHAIN:
    m_swap_chain_owner.remove(rid);
    break;
  case ResourceType::BUFFER:
    m_buffers_owner.remove(rid);
    break;
  case ResourceType::IMAGE:
    m_image_registry.remove(rid);
    break;
  case ResourceType::IMAGE_VIEW:
    m_image_view_registry.remove(rid);
    break;
  case ResourceType::PIPELINE:
    m_pipelines_owner.remove(rid);
    break;
  case ResourceType::DESCRIPTOR_SET_LAYOUT:
    m_ds_layout_owner.remove(rid);
    break;
  case ResourceType::PIPELINE_LAYOUT:
    m_pl_layout_owner.remove(rid);
    break;
  default:
    // Optional: log an error for an unhandled resource type
    break;
  }

  m_rid_type_map.erase(it);
}

} // namespace ssme::vulkan
