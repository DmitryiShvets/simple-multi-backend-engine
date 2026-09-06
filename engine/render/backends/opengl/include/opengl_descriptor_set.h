#pragma once

#include "core/render_types.h"
#include <glad/gl.h>
#include <vector>
#include <cstdint>

namespace ssme::opengl {

// Descriptor binding - links UBO/texture to binding point
struct DescriptorBinding {
    GLuint handle;        // Resource handle (UBO, texture, etc.)
    uint32_t binding;     // Binding point (must match shader!)
    bool is_texture = false;
};

// OpenGL DescriptorSet - emulates Vulkan DescriptorSet
// Stores collection of bindings and binds them to OpenGL context
class OpenGLDescriptorSet {
public:
    OpenGLDescriptorSet() = default;
    ~OpenGLDescriptorSet() = default;

    // Add binding
    void addBinding(uint32_t binding, GLuint handle, bool is_texture = false);

    // Bind all resources to OpenGL context
    void bind(uint32_t bind_point) const;

    // Get binding count
    size_t getBindingCount() const { return m_bindings.size(); }

    // Get binding by index
    const DescriptorBinding& getBinding(size_t index) const { return m_bindings[index]; }

private:
    std::vector<DescriptorBinding> m_bindings;
};

// Descriptor Set Layout for OpenGL
// Stores metadata about DescriptorSet structure (for validation)
class OpenGLDescriptorSetLayout {
public:
    struct BindingDesc {
        uint32_t binding;
        uint32_t type;  // DescriptorType
        uint32_t stages; // ShaderStageFlags
        uint32_t count;
    };

    OpenGLDescriptorSetLayout() = default;

    // Add binding description
    void addBinding(uint32_t binding, uint32_t type, uint32_t stages, uint32_t count = 1);

    // Get binding count
    size_t getBindingCount() const { return m_bindings.size(); }

    // Get binding by index
    const BindingDesc& getBinding(size_t index) const { return m_bindings[index]; }

private:
    std::vector<BindingDesc> m_bindings;
};

} // namespace ssme::opengl
