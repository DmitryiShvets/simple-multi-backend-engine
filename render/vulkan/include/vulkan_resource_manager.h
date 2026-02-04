#pragma once

#include <atomic>
#include <memory>
#include <type_traits>
#include <vulkan/vulkan.h>

#include "render_types.h"
#include "resource_owner.h"
#include "types.h"

// Forward-declarations for Vulkan implementation classes
namespace Render::Vulkan {
class VulkanSwapChain;
class VulkanBuffer;
// Add other resource types here as they are created
} // namespace Render::Vulkan

namespace Render::Vulkan {

class VulkanResourceManager {
public:
  VulkanResourceManager();
  ~VulkanResourceManager();

  // Overload 1: Takes ownership of a heap-allocated C++ object.
  template <typename T> RID add(std::unique_ptr<T> resource) {
    uint64_t id = m_next_rid.fetch_add(1);
    RID rid{id}; // Corrected initialization

    ResourceType type = ResourceType::UNDEFINED; // Corrected case

    // Use if constexpr to select the correct owner at compile time.
    if constexpr (std::is_same_v<T, VulkanSwapChain>) {
      type = ResourceType::SWAP_CHAIN; // Corrected case
      m_swap_chain_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanBuffer>) {
      type = ResourceType::BUFFER; // Corrected case
      // m_buffer_owner.insert(rid, std::move(resource));
    } else {
      // This will cause a compile error if you try to add an unsupported
      // resource type.
      static_assert(sizeof(T) < 0,
                    "Unsupported resource type in VulkanResourceManager");
    }

    m_rid_type_map[rid] = type;
    return rid;
  }

  // Overload 2: Registers an unowned, native handle.
  template <typename T> RID add(T handle) {
    uint64_t id = m_next_rid.fetch_add(1);
    RID rid{id}; // Corrected initialization

    ResourceType type = ResourceType::UNDEFINED; // Corrected case

    if constexpr (std::is_same_v<T, VkImage>) {
      type = ResourceType::TEXTURE; // Corrected case
      m_image_registry.insert(rid, handle);
    } else {
      static_assert(sizeof(T) < 0, "Unsupported handle type for unowned "
                                   "resources in VulkanResourceManager");
    }

    m_rid_type_map[rid] = type;
    return rid;
  }

  // Unified, type-safe getter for all resource types.
  template <typename T> T *get_ptr(RID rid) {
    if constexpr (std::is_same_v<T, VulkanSwapChain>) {
      return m_swap_chain_owner.get(rid);
    } else if constexpr (std::is_same_v<T, VulkanBuffer>) {
      // return m_buffer_owner.get(rid);
      return nullptr;
    }
    return nullptr;
  }
  template <typename T> T get_val(RID rid) {
    if constexpr (std::is_same_v<T, VkImage>) {
      return m_image_registry.get(rid);
    }
    return nullptr;
  }

  // Atomically frees a resource regardless of its type.
  void free(RID rid);

private:
  std::atomic<uint64_t> m_next_rid;
  std::unordered_map<RID, ResourceType> m_rid_type_map;

  // --- Owned Resources ---
  Core::ResourceOwner<VulkanSwapChain> m_swap_chain_owner;

  // --- Unowned (Registered) Resources ---
  Core::ResourceRegistry<VkImage> m_image_registry;
};

} // namespace Render::Vulkan
