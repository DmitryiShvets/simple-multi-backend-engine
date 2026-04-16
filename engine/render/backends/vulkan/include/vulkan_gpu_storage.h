#pragma once

#include "core/resource_owner.h"
#include "core/rid.h"
#include "core/rid_allocator.h"
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

// Full type definitions required for ResourceOwner< T> (uses unique_ptr
// internally)
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_pipeline.h"
#include "vulkan_pipeline_layout.h"
#include "vulkan_texture.h"
#include "vulkan_shader_module.h"

// Forward declarations for remaining Vulkan resources
namespace ssme::vulkan {
class VulkanImage;
class VulkanImageView;
class VulkanSampler;
class VulkanDescriptorPool;
// class VulkanShaderModule;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanFramebuffer;
} // namespace ssme::vulkan

namespace ssme {
class Material;
}

namespace ssme::vulkan {

template <typename T> struct GpuResourceTraits;

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
    getOwner<T>().insert(rid, std::move(resource));
  }

  /**
   * @brief Get a Vulkan resource by RID
   */
  template <typename T> T *get(RID rid) {
    LockGuard lock;
    return getOwner<T>().get(rid);
  }

  /**
   * @brief Get a Vulkan resource by RID (const version)
   */
  template <typename T> const T *get(RID rid) const {
    LockGuard lock;
    return getOwner<T>().get(rid);
  }

  /**
   * @brief Remove a resource by RID
   */
  template <typename T> bool remove(RID rid) {
    LockGuard lock;
    auto resource = getOwner<T>().remove(rid);
    return resource != nullptr;
  }

  /**
   * @brief Check if a resource exists
   */
  template <typename T> bool has(RID rid) {
    LockGuard lock;
    return getOwner<T>().get(rid) != nullptr;
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
  RID findPSO(std::size_t hash) {
    LockGuard lock;
    auto it = m_pso_map.find(hash);
    if (it != m_pso_map.end()) {
      return it->second;
    }
    return RID::INVALID;
  }

  /**
   * @brief Register PSO (Pipeline) with a name
   */
  void registerPSO(std::size_t name, RID rid) {
    LockGuard lock;
    m_pso_map[name] = rid;
  }

  /**
   * @brief Find Pipeline layout by hash
   */
  RID findPSOLayout(std::size_t hash) {
    LockGuard lock;
    auto it = m_pso_layout_map.find(hash);
    if (it != m_pso_layout_map.end()) {
      return it->second;
    }
    return RID::INVALID;
  }

  /**
   * @brief Register Material with a name
   */
  void registerPSOLayout(std::size_t hash, RID rid) {
    LockGuard lock;
    m_pso_layout_map[hash] = rid;
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
  ResourceOwner<VulkanShaderModule> m_shader_modules;

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

  std::unordered_map<std::size_t, RID> m_pso_map;
  std::unordered_map<std::size_t, RID> m_pso_layout_map;

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

  template <typename T> auto &getOwner() {
    constexpr auto ptr = GpuResourceTraits<T>::template member<THREAD_SAFE>;
    return (this->*ptr);
  }

  template <typename T> const auto &getOwner() const {
    constexpr auto ptr = GpuResourceTraits<T>::template member<THREAD_SAFE>;
    return (this->*ptr);
  }
  // Friend helper structs to access private members
  template <typename T> friend struct GpuResourceTraits;
};

// ============================================================================
// Helper struct for type-to-member mapping (with specializations)
// ============================================================================

#define REGISTER_VK_GPU_RESOURCE(Type, MemberName)                                \
  template <> struct GpuResourceTraits<Type> {                                 \
    template <bool TS>                                                         \
    static constexpr auto member = &VulkanGpuStorage<TS>::MemberName;          \
  }

REGISTER_VK_GPU_RESOURCE(VulkanSwapChain, m_swap_chains);
REGISTER_VK_GPU_RESOURCE(VulkanBuffer, m_buffers);
REGISTER_VK_GPU_RESOURCE(VulkanTexture, m_textures);
REGISTER_VK_GPU_RESOURCE(VulkanPipeLine, m_pipelines);
REGISTER_VK_GPU_RESOURCE(VulkanDescriptorSetLayout, m_ds_layouts);
REGISTER_VK_GPU_RESOURCE(VulkanDescriptorPool, m_descriptor_pools);
REGISTER_VK_GPU_RESOURCE(VulkanDescriptorSet, m_descriptor_sets);
REGISTER_VK_GPU_RESOURCE(VulkanPipelineLayout, m_pipeline_layouts);
REGISTER_VK_GPU_RESOURCE(VulkanShaderModule, m_shader_modules);

} // namespace ssme::vulkan
