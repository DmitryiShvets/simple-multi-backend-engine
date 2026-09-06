#include "opengl_descriptor_set.h"
#include <cstdint>

namespace ssme::opengl {

// *************** OpenGLDescriptorSet *********************

void OpenGLDescriptorSet::addBinding(uint32_t binding, GLuint handle,
                                     bool is_texture) {
  m_bindings.push_back({handle, binding, is_texture});
}

void OpenGLDescriptorSet::bind(uint32_t bind_point) const {
  uint32_t tex_unit = bind_point;
  // Bind each resource to its binding point
  for (const auto &b : m_bindings) {
    if (b.is_texture) {
      glActiveTexture(GL_TEXTURE0 + tex_unit);
      glBindTexture(GL_TEXTURE_2D, b.handle);
      tex_unit++;
    } else {
      // For UBO use glBindBufferBase
      // binding = 0, 1, 2... (must match layout(binding = X) in shader!)
      glBindBufferBase(GL_UNIFORM_BUFFER, bind_point, b.handle);
    }
  }
}

// *************** OpenGLDescriptorSetLayout *********************

void OpenGLDescriptorSetLayout::addBinding(uint32_t binding, uint32_t type,
                                           uint32_t stages, uint32_t count) {
  m_bindings.push_back({binding, type, stages, count});
}

} // namespace ssme::opengl
