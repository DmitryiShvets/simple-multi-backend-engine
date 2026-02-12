#pragma once

#include "resource_owner.h"
#include "resource_types.h"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_pipeline.h"
#include "vulkan_pipeline_layout.h"
#include "vulkan_texture.h"

#include <atomic>
#include <memory>
#include <string>
#include <vulkan/vulkan.h>

// Forward-declarations for Vulkan implementation classes
namespace Render::Vulkan {
class VulkanSwapChain;
class VulkanDataBuffer;
class VulkanTexture;
// Other classes are now included directly
} // namespace Render::Vulkan

namespace Render::Vulkan {

class VulkanResourceManager {
public:
  VulkanResourceManager();
  ~VulkanResourceManager();

  // Overload 1: Takes ownership of a heap-allocated C++ object.
  template <typename T> RID add(std::unique_ptr<T> resource) {
    uint64_t id = m_next_rid.fetch_add(1);
    RID rid{id};

    ResourceType type = ResourceType::UNDEFINED;

    if constexpr (std::is_same_v<T, VulkanSwapChain>) {
      type = ResourceType::SWAP_CHAIN;
      m_swap_chain_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanDataBuffer>) {
      type = ResourceType::BUFFER;
      m_buffers_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanTexture>) {
      type = ResourceType::TEXTURE;
      m_textures_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanPipeLine>) {
      type = ResourceType::PIPELINE;
      m_pipelines_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, DescriptorSetLayout>) {
      type = ResourceType::DESCRIPTOR_SET_LAYOUT;
      m_ds_layout_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanPipelineLayout>) {
      type = ResourceType::PIPELINE_LAYOUT;
      m_pl_layout_owner.insert(rid, std::move(resource));
    } else {
      static_assert(sizeof(T) < 0,
                    "Unsupported resource type in VulkanResourceManager");
    }

    m_rid_type_map[rid] = type;
    return rid;
  }

  // Overload 2: Registers an unowned, native handle.
  template <typename T> RID add(T handle) {
    uint64_t id = m_next_rid.fetch_add(1);
    RID rid{id};

    ResourceType type = ResourceType::UNDEFINED;

    if constexpr (std::is_same_v<T, VkImage>) {
      type = ResourceType::IMAGE;
      m_image_registry.insert(rid, handle);
    } else if constexpr (std::is_same_v<T, VkImageView>) {
      type = ResourceType::IMAGE_VIEW; // Re-use texture type for views
      m_image_view_registry.insert(rid, handle);
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
    } else if constexpr (std::is_same_v<T, VulkanDataBuffer>) {
      return m_buffers_owner.get(rid);
    } else if constexpr (std::is_same_v<T, VulkanTexture>) {
      return m_textures_owner.get(rid);
    } else if constexpr (std::is_same_v<T, VulkanPipeLine>) {
      return m_pipelines_owner.get(rid);
    } else if constexpr (std::is_same_v<T, DescriptorSetLayout>) {
      return m_ds_layout_owner.get(rid);
    } else if constexpr (std::is_same_v<T, VulkanPipelineLayout>) {
      return m_pl_layout_owner.get(rid);
    }
    return nullptr;
  }
  template <typename T> T get_val(RID rid) {
    if constexpr (std::is_same_v<T, VkImage>) {
      return m_image_registry.get(rid);
    } else if constexpr (std::is_same_v<T, VkImageView>) {
        return m_image_view_registry.get(rid);
    }
    return nullptr;
  }

  // Atomically frees a resource regardless of its type.
  void free(RID rid);

  // --- PSO (Pipeline) Management ---
  RID findPSO(const std::string &name) {
    auto it = m_pso_map.find(name);
    if (it != m_pso_map.end()) {
      return it->second;
    }
    return RID{}; // Return invalid RID if not found
  }

  void registerPSO(const std::string &name, RID rid) { m_pso_map[name] = rid; }

private:
  std::atomic<uint64_t> m_next_rid;
  std::unordered_map<RID, ResourceType> m_rid_type_map;
  std::unordered_map<std::string, RID> m_pso_map;

  // --- Owned Resources ---
  Core::ResourceOwner<VulkanSwapChain> m_swap_chain_owner;
  Core::ResourceOwner<VulkanDataBuffer> m_buffers_owner;
  Core::ResourceOwner<VulkanTexture> m_textures_owner;
  Core::ResourceOwner<VulkanPipeLine> m_pipelines_owner;
  Core::ResourceOwner<DescriptorSetLayout> m_ds_layout_owner;
  Core::ResourceOwner<VulkanPipelineLayout> m_pl_layout_owner;

  // --- Unowned (Registered) Resources ---
  Core::ResourceRegistry<VkImage> m_image_registry;
  Core::ResourceRegistry<VkImageView> m_image_view_registry;
};

} // namespace Render::Vulkan
