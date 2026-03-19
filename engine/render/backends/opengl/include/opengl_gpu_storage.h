#pragma once

#include "core/rid.h"
#include <memory>
#include <unordered_map>
#include <mutex>
#include <any>

// Forward declarations for OpenGL resources
namespace ssme::opengl {
class OpenGLBuffer;
class OpenGLTexture;
class OpenGLShaderProgram;
class OpenGLVertexArray;
class OpenGLFramebuffer;
class OpenGLRenderbuffer;
class OpenGLSampler;
class OpenGLUniformBuffer;
class OpenGLDescriptorSet;
class OpenGLDescriptorSetLayout;
} // namespace ssme::opengl

namespace ssme::opengl {

/**
 * @brief Storage for OpenGL GPU resources keyed by external RID
 * 
 * Does NOT generate RIDs - accepts them from central ResourceManager.
 * Stores per-backend OpenGL resources (GLuint buffers, textures, etc.)
 * 
 * @tparam THREAD_SAFE Enable mutex protection (default: true)
 * 
 * Usage:
 *   OpenGLGpuStorage storage;
 *   RID rid = externalAllocator.allocate();  // From ResourceManager
 *   storage.store(rid, std::make_unique<OpenGLBuffer>(...));
 *   auto* buffer = storage.get<OpenGLBuffer>(rid);
 */
template<bool THREAD_SAFE = true>
class OpenGLGpuStorage {
public:
    OpenGLGpuStorage() = default;
    ~OpenGLGpuStorage() = default;
    
    // Non-copyable, non-movable
    OpenGLGpuStorage(const OpenGLGpuStorage&) = delete;
    OpenGLGpuStorage& operator=(const OpenGLGpuStorage&) = delete;
    OpenGLGpuStorage(OpenGLGpuStorage&&) = delete;
    OpenGLGpuStorage& operator=(OpenGLGpuStorage&&) = delete;

    /**
     * @brief Store an OpenGL resource with external RID
     * @tparam T Resource type (OpenGLBuffer, OpenGLTexture, etc.)
     * @param rid Resource ID (from central ResourceManager)
     * @param resource Resource to store (takes ownership)
     */
    template<typename T>
    void store(RID rid, std::unique_ptr<T> resource) {
        LockGuard lock;
        m_resources[rid.id] = std::move(resource);
    }

    /**
     * @brief Get an OpenGL resource by RID
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

using OpenGLGpuStorageMT = OpenGLGpuStorage<true>;   // Thread-safe (default)
using OpenGLGpuStorageST = OpenGLGpuStorage<false>;  // Single-threaded (faster)

} // namespace ssme::opengl
