#pragma once

#include "core/rid.h"
#include <memory>
#include <unordered_map>
#include <mutex>
#include <any>

// Forward declarations for Vulkan resources
namespace ssme::vulkan {
class VulkanBuffer;
class VulkanTexture;
class VulkanImage;
class VulkanImageView;
class VulkanSampler;
class VulkanPipeline;
class VulkanPipelineLayout;
class VulkanDescriptorSetLayout;
class VulkanDescriptorSet;
class VulkanDescriptorPool;
class VulkanShaderModule;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanFramebuffer;
} // namespace ssme::vulkan

namespace ssme::vulkan {

/**
 * @brief Storage for Vulkan GPU resources keyed by external RID
 * 
 * Does NOT generate RIDs - accepts them from central ResourceManager.
 * Stores per-backend Vulkan resources (VkBuffer, VkImage, etc.)
 * 
 * @tparam THREAD_SAFE Enable mutex protection (default: true)
 * 
 * Usage:
 *   VulkanGpuStorage storage;
 *   RID rid = externalAllocator.allocate();  // From ResourceManager
 *   storage.store(rid, std::make_unique<VulkanBuffer>(...));
 *   auto* buffer = storage.get<VulkanBuffer>(rid);
 */
template<bool THREAD_SAFE = true>
class VulkanGpuStorage {
public:
    VulkanGpuStorage() = default;
    ~VulkanGpuStorage() = default;
    
    // Non-copyable, non-movable
    VulkanGpuStorage(const VulkanGpuStorage&) = delete;
    VulkanGpuStorage& operator=(const VulkanGpuStorage&) = delete;
    VulkanGpuStorage(VulkanGpuStorage&&) = delete;
    VulkanGpuStorage& operator=(VulkanGpuStorage&&) = delete;

    /**
     * @brief Store a Vulkan resource with external RID
     * @tparam T Resource type (VulkanBuffer, VulkanTexture, etc.)
     * @param rid Resource ID (from central ResourceManager)
     * @param resource Resource to store (takes ownership)
     */
    template<typename T>
    void store(RID rid, std::unique_ptr<T> resource) {
        LockGuard lock;
        m_resources[rid.id] = std::move(resource);
    }

    /**
     * @brief Get a Vulkan resource by RID
     * @tparam T Resource type
     * @param rid Resource ID
     * @return Pointer to resource, or nullptr if not found
     */
    template<typename T>
    T* get(RID rid) const {
        LockGuard lock;
        
        auto it = m_resources.find(rid.id);
        if (it == m_resources.end()) {
            return nullptr;
        }
        
        try {
            return std::any_cast<T*>(it->second);
        } catch (const std::bad_any_cast&) {
            return nullptr;  // Wrong type
        }
    }

    /**
     * @brief Remove a resource by RID
     * @param rid Resource ID to remove
     * @return true if removed, false if not found
     */
    bool remove(RID rid) {
        LockGuard lock;
        return m_resources.erase(rid.id) > 0;
    }

    /**
     * @brief Check if a resource exists
     * @param rid Resource ID
     * @return true if exists
     */
    bool has(RID rid) const {
        LockGuard lock;
        return m_resources.contains(rid.id);
    }

    /**
     * @brief Get count of stored resources
     */
    size_t count() const {
        LockGuard lock;
        return m_resources.size();
    }

    /**
     * @brief Clear all resources
     */
    void clear() {
        LockGuard lock;
        m_resources.clear();
    }

    /**
     * @brief Reserve capacity (optimization)
     * @param expected_max Expected maximum number of resources
     */
    void reserve(size_t expected_max) {
        LockGuard lock;
        m_resources.reserve(expected_max);
    }

private:
    /// RID.id → typed resource pointer (stored as std::any)
    std::unordered_map<uint64_t, std::any> m_resources;

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
};

// ============================================================================
// Convenience typedefs
// ============================================================================

using VulkanGpuStorageMT = VulkanGpuStorage<true>;   // Thread-safe (default)
using VulkanGpuStorageST = VulkanGpuStorage<false>;  // Single-threaded (faster)

} // namespace ssme::vulkan
