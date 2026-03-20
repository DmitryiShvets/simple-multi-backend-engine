#pragma once

#include "core/resource_owner.h"
#include "core/rid.h"
#include "core/rid_allocator.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

// Full type definitions required for ResourceOwner<T> (uses unique_ptr
// internally)
#include "opengl_buffer_objects.h"
#include "opengl_descriptor_set.h"
#include "opengl_shader_program.h"

namespace ssme {
class Material; // Forward declare from parent namespace
}

// Forward declarations for remaining OpenGL resources (not yet used)
namespace ssme::opengl {
class OpenGLRenderbuffer;
class OpenGLSampler;
} // namespace ssme::opengl

namespace ssme::opengl {

template <typename T> struct GpuResourceTraits;

// ============================================================================
// OpenGLGpuStorage - Main storage class
// ============================================================================

/**
 * @brief Storage for OpenGL GPU resources keyed by RID
 *
 * Uses ResourceOwner<T> for each resource type (type-safe, no std::any).
 * Supports both internal resources (auto-generated RID) and external
 * resources (RID from central ResourceManager).
 *
 * @tparam THREAD_SAFE Enable mutex protection (default: true)
 */
template <bool THREAD_SAFE = true> class OpenGLGpuStorage {
public:
  OpenGLGpuStorage() = default;
  ~OpenGLGpuStorage() = default;

  // Non-copyable, non-movable
  OpenGLGpuStorage(const OpenGLGpuStorage &) = delete;
  OpenGLGpuStorage &operator=(const OpenGLGpuStorage &) = delete;
  OpenGLGpuStorage(OpenGLGpuStorage &&) = delete;
  OpenGLGpuStorage &operator=(OpenGLGpuStorage &&) = delete;

  /**
   * @brief Add an internal OpenGL resource (generates RID automatically)
   */
  template <typename T> RID add(std::unique_ptr<T> resource) {
    RID rid = m_rid_allocator.allocate();
    store<T>(rid, std::move(resource));
    return rid;
  }

  /**
   * @brief Store an OpenGL resource with external RID
   */
  template <typename T> void store(RID rid, std::unique_ptr<T> resource) {
    LockGuard lock;
    getOwner<T>().insert(rid, std::move(resource));
  }

  /**
   * @brief Get an OpenGL resource by RID
   */
  template <typename T> T *get(RID rid) {
    LockGuard lock;
    return getOwner<T>().get(rid);
  }

  /**
   * @brief Get an OpenGL resource by RID (const version)
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
    // ResourceOwner doesn't have clear()
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

  ResourceOwner<ShaderProgram> m_shader_programs;
  ResourceOwner<VAO> m_buffers;
  ResourceOwner<ssme::Material> m_materials;
  ResourceOwner<UniformBuffer> m_uniform_buffers;
  ResourceOwner<OpenGLDescriptorSet> m_descriptor_sets;
  ResourceOwner<OpenGLDescriptorSetLayout> m_descriptor_set_layouts;

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

#define REGISTER_GL_GPU_RESOURCE(Type, MemberName)                                \
  template <> struct GpuResourceTraits<Type> {                                 \
    template <bool TS>                                                         \
    static constexpr auto member = &OpenGLGpuStorage<TS>::MemberName;          \
  }

REGISTER_GL_GPU_RESOURCE(ShaderProgram, m_shader_programs);
REGISTER_GL_GPU_RESOURCE(VAO, m_buffers);
REGISTER_GL_GPU_RESOURCE(UniformBuffer, m_uniform_buffers);
REGISTER_GL_GPU_RESOURCE(OpenGLDescriptorSet, m_descriptor_sets);
REGISTER_GL_GPU_RESOURCE(OpenGLDescriptorSetLayout, m_descriptor_set_layouts);
REGISTER_GL_GPU_RESOURCE(ssme::Material, m_materials);

} // namespace ssme::opengl
