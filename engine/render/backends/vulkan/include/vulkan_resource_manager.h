#pragma once

#include "core/resource_owner.h"
#include "core/resource_types.h"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_pipeline.h"
#include "vulkan_pipeline_layout.h"
#include "vulkan_texture.h"

#include <atomic>
#include <memory>
#include <string>

// Forward-declarations for Vulkan implementation classes
namespace ssme::vulkan {
class VulkanSwapChain;
class VulkanBuffer;
class VulkanTexture;
class VulkanPipeLine;
class VulkanDescriptorSetLayout;
class VulkanDescriptorPool;
class VulkanPipelineLayout;
class VulkanDescriptorSet;
// Add other resource types here as they are created
} // namespace ssme::vulkan

namespace ssme::vulkan {

class VulkanResourceManager {
public:
  VulkanResourceManager();
  ~VulkanResourceManager();

  // Overload 1: Takes ownership of a heap-allocated C++ object.
  template <typename T> RID add(std::unique_ptr<T> resource) {
    // Start RID from 1, because RID::id=0 represents invalid/null resource
    uint64_t id = m_next_rid.fetch_add(1) + 1;
    RID rid{id};

    ResourceType type = ResourceType::UNDEFINED;

    if constexpr (std::is_same_v<T, VulkanSwapChain>) {
      type = ResourceType::SWAP_CHAIN;
      m_swap_chain_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanBuffer>) {
      type = ResourceType::BUFFER;
      m_buffers_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanTexture>) {
      type = ResourceType::TEXTURE;
      m_textures_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanPipeLine>) {
      type = ResourceType::PIPELINE;
      m_pipelines_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanDescriptorSetLayout>) {
      type = ResourceType::DESCRIPTOR_SET_LAYOUT;
      m_ds_layout_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanPipelineLayout>) {
      type = ResourceType::PIPELINE_LAYOUT;
      m_pl_layout_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanDescriptorPool>) {
      type = ResourceType::DESCRIPTOR_POOL;
      m_descriptor_pool_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, Material>) {
      type = ResourceType::MATERIAL_TEMPLATE;
      m_materials_owner.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VulkanDescriptorSet>) {
      type = ResourceType::DESCRIPTOR_SET;
      m_descriptor_set_owner.insert(rid, std::move(resource));
    } else {
      static_assert(sizeof(T) < 0,
                    "Unsupported resource type in VulkanResourceManager");
    }

    m_rid_type_map[rid] = type;
    return rid;
  }

  // Overload 2: Registers an unowned, native handle.
  template <typename T> RID add(T handle) {
    // Start RID from 1, because RID::id=0 represents invalid/null resource
    uint64_t id = m_next_rid.fetch_add(1) + 1;
    RID rid{id};

    ResourceType type = ResourceType::UNDEFINED;

    if constexpr (std::is_same_v<T, vk::Image>) {
      type = ResourceType::IMAGE;
      m_image_registry.insert(rid, handle);
    } else if constexpr (std::is_same_v<T, vk::ImageView>) {
      type = ResourceType::IMAGE_VIEW;
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
    } else if constexpr (std::is_same_v<T, VulkanBuffer>) {
      return m_buffers_owner.get(rid);
    } else if constexpr (std::is_same_v<T, VulkanTexture>) {
      return m_textures_owner.get(rid);
    } else if constexpr (std::is_same_v<T, VulkanPipeLine>) {
      return m_pipelines_owner.get(rid);
    } else if constexpr (std::is_same_v<T, VulkanDescriptorSetLayout>) {
      return m_ds_layout_owner.get(rid);
    } else if constexpr (std::is_same_v<T, VulkanPipelineLayout>) {
      return m_pl_layout_owner.get(rid);
    } else if constexpr (std::is_same_v<T, Material>) {
      return m_materials_owner.get(rid);
    } else if constexpr (std::is_same_v<T, VulkanDescriptorSet>) {
      return m_descriptor_set_owner.get(rid);
    }
    return nullptr;
  }

  template <typename T> T get_val(RID rid) {
    if constexpr (std::is_same_v<T, vk::Image>) {
      return m_image_registry.get(rid);
    } else if constexpr (std::is_same_v<T, vk::ImageView>) {
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

  // --- Material Management ---
  RID findMaterial(const std::string &name) {
    auto it = m_mat_map.find(name);
    if (it != m_mat_map.end()) {
      return it->second;
    }
    return RID{}; // Return invalid RID if not found
  }

  void registerMaterial(const std::string &name, RID rid) {
    m_mat_map[name] = rid;
  }

private:
  std::atomic<uint64_t> m_next_rid;
  std::unordered_map<RID, ResourceType> m_rid_type_map;
  std::unordered_map<std::string, RID> m_pso_map;
  std::unordered_map<std::string, RID> m_mat_map;

  // --- Owned Resources ---
  ResourceOwner<VulkanSwapChain> m_swap_chain_owner;
  ResourceOwner<VulkanBuffer> m_buffers_owner;
  ResourceOwner<VulkanTexture> m_textures_owner;
  ResourceOwner<VulkanPipeLine> m_pipelines_owner;
  ResourceOwner<VulkanDescriptorSetLayout> m_ds_layout_owner;
  ResourceOwner<VulkanDescriptorPool> m_descriptor_pool_owner;
  ResourceOwner<VulkanPipelineLayout> m_pl_layout_owner;
  ResourceOwner<Material> m_materials_owner;
  ResourceOwner<VulkanDescriptorSet> m_descriptor_set_owner;

  // --- Unowned (Registered) Resources ---
  ResourceRegistry<vk::Image> m_image_registry;
  ResourceRegistry<vk::ImageView> m_image_view_registry;
};

} // namespace ssme::vulkan
