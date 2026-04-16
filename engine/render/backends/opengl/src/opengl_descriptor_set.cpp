#include "opengl_descriptor_set.h"
#include <cstdint>

namespace ssme::opengl {

// *************** OpenGLDescriptorSet *********************

void OpenGLDescriptorSet::addBinding(uint32_t binding, GLuint handle) {
    m_bindings.push_back({handle, binding});
}

void OpenGLDescriptorSet::bind(uint32_t bind_point) const {
    // Bind each resource to its binding point
    for (const auto& b : m_bindings) {
        // For UBO use glBindBufferBase
        // binding = 0, 1, 2... (must match layout(binding = X) in shader!)
        glBindBufferBase(GL_UNIFORM_BUFFER, bind_point, b.handle);
    }
}

// *************** OpenGLDescriptorSetLayout *********************

void OpenGLDescriptorSetLayout::addBinding(uint32_t binding, uint32_t type,
                                            uint32_t stages, uint32_t count) {
    m_bindings.push_back({binding, type, stages, count});
}

} // namespace ssme::opengl
