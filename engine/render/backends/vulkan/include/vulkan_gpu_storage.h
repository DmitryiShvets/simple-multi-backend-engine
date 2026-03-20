#pragma once

#include "core/resource_owner.h"
#include "core/rid.h"
#include "core/rid_allocator.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

// Full type definitions required for ResourceOwner< T> (uses unique_ptr internally)
#include "vulkan_buffer.h"
#include "vulkan_texture.h"
#include "vulkan_pipeline.h"
#include "vulkan_pipeline_layout.h"
#include "vulkan_descriptor_set.h"

// Forward declarations for remaining Vulkan resources
namespace ssme::vulkan {
class VulkanImage;
class VulkanImageView;
class VulkanSampler;
class VulkanDescriptorPool;
class VulkanShaderModule;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanFramebuffer;
} // namespace ssme::vulkan

namespace ssme {
class Material;
}

namespace ssme::vulkan {

template <typename T> struct GpuStorageHelper;

// ============================================================================
// VulkanGpuStorage - Main storage class
// ============================================================================

/**
 * @brief Storage for Vulkan GPU resources keyed by RID
 *
 * Uses ResourceOwner<T> for each resource type (type-safe, no std::any).
 * Supports both internal resources (auto-generated RID) and external
 * resources (RID from central ResourceManager).
 *
 * @tparam THREAD_SAFE Enable mutex protection (default: true)
 */
template <bool THREAD_SAFE = true> class VulkanGpuStorage {
public:
  VulkanGpuStorage() = default;
  ~VulkanGpuStorage() = default;

  // Non-copyable, non-movable
  VulkanGpuStorage(const VulkanGpuStorage &) = delete;
  VulkanGpuStorage &operator=(const VulkanGpuStorage &) = delete;
  VulkanGpuStorage(VulkanGpuStorage &&) = delete;
  VulkanGpuStorage &operator=(VulkanGpuStorage &&) = delete;

  /**
   * @brief Add an internal Vulkan resource (generates RID automatically)
   */
  template <typename T> RID add(std::unique_ptr<T> resource) {
    RID rid = m_rid_allocator.allocate();
    store<T>(rid, std::move(resource));
    return rid;
  }

  /**
   * @brief Store a Vulkan resource with external RID
   */
  template <typename T> void store(RID rid, std::unique_ptr<T> resource) {
    LockGuard lock;
    GpuStorageHelper<T>::get(*this).insert(rid, std::move(resource));
  }

  /**
   * @brief Get a Vulkan resource by RID
   */
  template <typename T> T *get(RID rid) {
    LockGuard lock;
    return GpuStorageHelper<T>::get(*this).get(rid);
  }

  /**
   * @brief Get a Vulkan resource by RID (const version)
   */
  template <typename T> const T *get(RID rid) const {
    LockGuard lock;
    return GpuStorageHelper<T>::get(*this).get(rid);
  }

  /**
   * @brief Remove a resource by RID
   */
  template <typename T> bool remove(RID rid) {
    LockGuard lock;
    auto resource = GpuStorageHelper<T>::get(*this).remove(rid);
    return resource != nullptr;
  }

  /**
   * @brief Check if a resource exists
   */
  template <typename T> bool has(RID rid) {
    LockGuard lock;
    return GpuStorageHelper<T>::get(*this).get(rid) != nullptr;
  }

  /**
   * @brief Clear all resources of a specific type
   */
  template <typename T> void clear() {
    // ResourceOwner doesn't have clear(), resources freed on destruction
  }

  /**
   * @brief Clear all resources (all types)
   */
  void clearAll() {
    // ResourceOwners will be destroyed automatically
  }

  /**
   * @brief Find PSO (Pipeline) by name
   */
  RID findPSO(const std::string &name) {
    LockGuard lock;
    auto it = m_pso_map.find(name);
    if (it != m_pso_map.end()) {
      return it->second;
    }
    return RID::INVALID;
  }

  /**
   * @brief Register PSO (Pipeline) with a name
   */
  void registerPSO(const std::string &name, RID rid) {
    LockGuard lock;
    m_pso_map[name] = rid;
  }

  /**
   * @brief Find Material by name
   */
  RID findMaterial(const std::string &name) {
    LockGuard lock;
    auto it = m_material_map.find(name);
    if (it != m_material_map.end()) {
      return it->second;
    }
    return RID::INVALID;
  }

  /**
   * @brief Register Material with a name
   */
  void registerMaterial(const std::string &name, RID rid) {
    LockGuard lock;
    m_material_map[name] = rid;
  }

private:
  // ========================================================================
  // Per-type ResourceOwner (type-safe storage)
  // ========================================================================

  ResourceOwner<VulkanSwapChain> m_swap_chains;
  ResourceOwner<VulkanBuffer> m_buffers;
  ResourceOwner<VulkanTexture> m_textures;
  ResourceOwner<VulkanPipeLine> m_pipelines;
  ResourceOwner<VulkanDescriptorSetLayout> m_ds_layouts;
  ResourceOwner<VulkanDescriptorPool> m_descriptor_pools;
  ResourceOwner<VulkanDescriptorSet> m_descriptor_sets;
  ResourceOwner<VulkanPipelineLayout> m_pipeline_layouts;
  ResourceOwner<ssme::Material> m_materials;

  // ========================================================================
  // RID Allocator for internal resources
  // ========================================================================

  /// Internal RID allocator (range: 1 to 1 trillion for backend-internal
  /// resources)
  RIDAllocator m_rid_allocator{RIDRange::INTERNAL_START,
                               RIDRange::INTERNAL_END};

  // ========================================================================
  // Name-to-RID maps for PSO and Materials
  // ========================================================================

  std::unordered_map<std::string, RID> m_pso_map;
  std::unordered_map<std::string, RID> m_material_map;

  // ========================================================================
  // Thread Safety
  // ========================================================================

  /// Mutex for thread safety (only used if THREAD_SAFE=true)
  mutable std::mutex m_mutex;

  /// Lock guard helper (empty if THREAD_SAFE=false)
  struct LockGuard {
#ifdef THREAD_SAFE
    LockGuard() : lock_(m_mutex) {}
    mutable std::lock_guard<std::mutex> lock_;
#else
    LockGuard() {}
#endif
  };

  // Friend helper structs to access private members
  template <typename T> friend struct GpuStorageHelper;
};

// ============================================================================
// Helper struct for type-to-member mapping (with specializations)
// ============================================================================

// Specializations for each supported type
template <> struct GpuStorageHelper<VulkanBuffer> {
  using OwnerType = ResourceOwner<VulkanBuffer>;
  static OwnerType &get(class VulkanGpuStorage<> &storage) {
    return storage.m_buffers;
  }
  static const OwnerType &get(const class VulkanGpuStorage<> &storage) {
    return storage.m_buffers;
  }
};

template <> struct GpuStorageHelper<VulkanTexture> {
  using OwnerType = ResourceOwner<VulkanTexture>;
  static OwnerType &get(class VulkanGpuStorage<> &storage) {
    return storage.m_textures;
  }
  static const OwnerType &get(const class VulkanGpuStorage<> &storage) {
    return storage.m_textures;
  }
};

template <> struct GpuStorageHelper<VulkanPipeLine> {
  using OwnerType = ResourceOwner<VulkanPipeLine>;
  static OwnerType &get(class VulkanGpuStorage<> &storage) {
    return storage.m_pipelines;
  }
  static const OwnerType &get(const class VulkanGpuStorage<> &storage) {
    return storage.m_pipelines;
  }
};

template <> struct GpuStorageHelper<VulkanPipelineLayout> {
  using OwnerType = ResourceOwner<VulkanPipelineLayout>;
  static OwnerType &get(class VulkanGpuStorage<> &storage) {
    return storage.m_pipeline_layouts;
  }
  static const OwnerType &get(const class VulkanGpuStorage<> &storage) {
    return storage.m_pipeline_layouts;
  }
};

template <> struct GpuStorageHelper<VulkanDescriptorSetLayout> {
  using OwnerType = ResourceOwner<VulkanDescriptorSetLayout>;
  static OwnerType &get(class VulkanGpuStorage<> &storage) {
    return storage.m_ds_layouts;
  }
  static const OwnerType &get(const class VulkanGpuStorage<> &storage) {
    return storage.m_ds_layouts;
  }
};

template <> struct GpuStorageHelper<VulkanDescriptorSet> {
  using OwnerType = ResourceOwner<VulkanDescriptorSet>;
  static OwnerType &get(class VulkanGpuStorage<> &storage) {
    return storage.m_descriptor_sets;
  }
  static const OwnerType &get(const class VulkanGpuStorage<> &storage) {
    return storage.m_descriptor_sets;
  }
};

template <> struct GpuStorageHelper<VulkanDescriptorPool> {
  using OwnerType = ResourceOwner<VulkanDescriptorPool>;
  static OwnerType &get(class VulkanGpuStorage<> &storage) {
    return storage.m_descriptor_pools;
  }
  static const OwnerType &get(const class VulkanGpuStorage<> &storage) {
    return storage.m_descriptor_pools;
  }
};

template <> struct GpuStorageHelper<VulkanSwapChain> {
  using OwnerType = ResourceOwner<VulkanSwapChain>;
  static OwnerType &get(class VulkanGpuStorage<> &storage) {
    return storage.m_swap_chains;
  }
  static const OwnerType &get(const class VulkanGpuStorage<> &storage) {
    return storage.m_swap_chains;
  }
};

template <> struct GpuStorageHelper<ssme::Material> {
  using OwnerType = ResourceOwner<ssme::Material>;
  static OwnerType &get(class VulkanGpuStorage<> &storage) {
    return storage.m_materials;
  }
  static const OwnerType &get(const class VulkanGpuStorage<> &storage) {
    return storage.m_materials;
  }
};
} // namespace ssme::vulkan
