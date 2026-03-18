#pragma once

#include "core/resource_owner.h"
#include "core/resource_types.h"

#include <atomic>

// Forward-declarations for OpenGL implementation classes
namespace ssme::opengl {
class ShaderProgram;
class VAO;
class UniformBuffer;
class OpenGLDescriptorSet;
class OpenGLDescriptorSetLayout;
// Add other resource types here as they are created
} // namespace ssme::opengl

namespace ssme::opengl {

class OpenglResourceManager {
public:
  OpenglResourceManager();
  ~OpenglResourceManager();
  OpenglResourceManager(const OpenglResourceManager &) = delete;
  OpenglResourceManager &operator=(const OpenglResourceManager &) = delete;
  OpenglResourceManager &operator=(OpenglResourceManager &&program) = delete;
  OpenglResourceManager(OpenglResourceManager &&program) = delete;
  void initialize();

  void destroy();

  // Overload 1: Takes ownership of a heap-allocated C++ object.
  template <typename T> RID add(std::unique_ptr<T> resource) {
    // Start RID from 1, because RID::id=0 represents invalid/null resource
    uint64_t id = m_next_rid.fetch_add(1) + 1;
    RID rid{id};

    ResourceType type = ResourceType::UNDEFINED;

    // Use if constexpr to select the correct owner at compile time.
    if constexpr (std::is_same_v<T, ShaderProgram>) {
      type = ResourceType::PIPELINE;
      m_programs.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, VAO>) {
      type = ResourceType::BUFFER;
      m_buffers.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, Material>) {
      type = ResourceType::MATERIAL_TEMPLATE;
      m_materials.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, UniformBuffer>) {
      type = ResourceType::BUFFER;
      m_uniform_buffers.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, OpenGLDescriptorSet>) {
      type = ResourceType::DESCRIPTOR_SET;
      m_descriptor_sets.insert(rid, std::move(resource));
    } else if constexpr (std::is_same_v<T, OpenGLDescriptorSetLayout>) {
      type = ResourceType::DESCRIPTOR_SET_LAYOUT;
      m_ds_layouts.insert(rid, std::move(resource));
    } else {
      // This will cause a compile error if you try to add an unsupported
      // resource type.
      static_assert(sizeof(T) < 0,
                    "Unsupported resource type in OpenglResourceManager");
    }

    m_rid_type_map[rid] = type;
    return rid;
  }

  // Unified, type-safe getter for all resource types.
  template <typename T> T *get_ptr(RID rid) {
    if constexpr (std::is_same_v<T, ShaderProgram>) {
      return m_programs.get(rid);
    } else if constexpr (std::is_same_v<T, VAO>) {
      return m_buffers.get(rid);
    } else if constexpr (std::is_same_v<T, Material>) {
      return m_materials.get(rid);
    } else if constexpr (std::is_same_v<T, UniformBuffer>) {
      return m_uniform_buffers.get(rid);
    } else if constexpr (std::is_same_v<T, OpenGLDescriptorSet>) {
      return m_descriptor_sets.get(rid);
    } else if constexpr (std::is_same_v<T, OpenGLDescriptorSetLayout>) {
      return m_ds_layouts.get(rid);
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

  void registerMaterial(const std::string &name, RID rid) { m_mat_map[name] = rid; }

private:
  std::atomic<uint64_t> m_next_rid;
  std::unordered_map<RID, ResourceType> m_rid_type_map;
  std::unordered_map<std::string, RID> m_pso_map;
  std::unordered_map<std::string, RID> m_mat_map;

  ResourceOwner<ShaderProgram> m_programs;
  ResourceOwner<VAO> m_buffers;
  ResourceOwner<Material> m_materials;
  ResourceOwner<UniformBuffer> m_uniform_buffers;
  ResourceOwner<OpenGLDescriptorSet> m_descriptor_sets;
  ResourceOwner<OpenGLDescriptorSetLayout> m_ds_layouts;
};
} // namespace ssme::opengl
