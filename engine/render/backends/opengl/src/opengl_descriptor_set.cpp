#include "opengl_descriptor_set.h"
#include "core/glsl_layout.h"

#include <cstdint>

namespace ssme::opengl {

// *************** OpenGLDescriptorSet *********************

void OpenGLDescriptorSet::addBinding(uint32_t binding, GLuint handle,
                                     bool is_texture) {
  m_bindings.push_back({handle, binding, is_texture});
}

void OpenGLDescriptorSet::bind(uint32_t bind_point) const {
  // Descriptor sets are flattened into a single GL binding namespace:
  //   glsl binding = reflected(binding) + set * kOpenglSetStride
  // Must match the GLSL patcher used by the Slang compiler.
  const uint32_t base = bind_point * kOpenglSetStride;
  for (const auto &b : m_bindings) {
    const uint32_t binding = base + b.binding;
    if (b.is_texture) {
      glActiveTexture(GL_TEXTURE0 + binding);
      glBindTexture(GL_TEXTURE_2D, b.handle);
    } else {
      // For UBO use glBindBufferBase
      // binding = 0, 1, 2... (must match layout(binding = X) in shader!)
      glBindBufferBase(GL_UNIFORM_BUFFER, binding, b.handle);
    }
  }
}

// *************** OpenGLDescriptorSetLayout *********************

void OpenGLDescriptorSetLayout::addBinding(uint32_t binding, uint32_t type,
                                           uint32_t stages, uint32_t count) {
  m_bindings.push_back({binding, type, stages, count});
}

} // namespace ssme::opengl
