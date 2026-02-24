#include "opengl_command_list.h"
#include "opengl_shader_program.h"
#include "opengl_descriptor_set.h"
#include "opengl_buffer_objects.h"

namespace Render::OpenGL {

OpenGLCommandList::OpenGLCommandList(OpenglResourceManager &resource_manager)
    : m_resource_manager(resource_manager) {}

// --- Pipeline State ---
void OpenGLCommandList::setGraphicsPipeline(RID pipeline_rid) {
  // Bind shader program immediately
  auto *program = m_resource_manager.get_ptr<ShaderProgram>(pipeline_rid);
  if (program) {
    program->use();
  }
}

void OpenGLCommandList::setViewport(const Viewport &viewport) {
  glViewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y),
             static_cast<GLsizei>(viewport.width),
             static_cast<GLsizei>(viewport.height));
  glDepthRange(viewport.minDepth, viewport.maxDepth);
}

void OpenGLCommandList::setScissor(const Rect &rect) {
  glScissor(static_cast<GLint>(rect.x), static_cast<GLint>(rect.y),
            static_cast<GLsizei>(rect.width),
            static_cast<GLsizei>(rect.height));
}

void OpenGLCommandList::setDepthBias(float constant_factor,
                                     float slope_factor) {
  if (constant_factor != 0.0f || slope_factor != 0.0f) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(slope_factor, constant_factor);
  } else {
    glDisable(GL_POLYGON_OFFSET_FILL);
  }
}

// --- Resource Binding ---
void OpenGLCommandList::setVertexBuffer(uint32_t first_binding, RID buffer_rid,
                                        uint64_t offset) {
  // In OpenGL, we bind the VAO which contains the VBO and vertex attribute state
  (void)first_binding;
  (void)offset;
  
  // Only bind if it's a VAO (vertex buffer), not a UniformBuffer
  auto* vao = m_resource_manager.get_ptr<VAO>(buffer_rid);
  if (vao) {
    vao->bind();
  }
}

void OpenGLCommandList::setIndexBuffer(RID buffer_rid, uint64_t offset,
                                       IndexType type) {
  (void)buffer_rid;
  (void)offset;
  (void)type;
}

void OpenGLCommandList::setDescriptorSet(uint32_t set_index, RID set_rid,
                                         RID pipeline_rid) {
  (void)set_index;
  (void)pipeline_rid;

  // Get DescriptorSet and bind all resources
  auto* desc_set = m_resource_manager.get_ptr<OpenGLDescriptorSet>(set_rid);
  if (desc_set) {
    desc_set->bind();  // Calls glBindBufferBase for each binding
  }
}

void OpenGLCommandList::setPushConstant(RID pipeline_rid,
                                        const UniformValue &value,
                                        ShaderStageFlags stages,
                                        uint32_t offset) {
  auto *program = m_resource_manager.get_ptr<ShaderProgram>(pipeline_rid);
  if (!program)
    return;
  program->setUniform(value);
}

// --- Drawing ---
void OpenGLCommandList::draw(uint32_t vertex_count, uint32_t instance_count,
                             uint32_t first_vertex, uint32_t first_instance) {
  if (instance_count > 1) {
    glDrawArraysInstanced(GL_TRIANGLES, first_vertex, vertex_count,
                          instance_count);
  } else {
    glDrawArrays(GL_TRIANGLES, first_vertex, vertex_count);
  }
}

void OpenGLCommandList::drawIndexed(uint32_t index_count,
                                    uint32_t instance_count,
                                    uint32_t first_index, int32_t vertex_offset,
                                    uint32_t first_instance) {
  // TODO: Implement indexed drawing
  (void)index_count;
  (void)instance_count;
  (void)first_index;
  (void)vertex_offset;
  (void)first_instance;
}

void OpenGLCommandList::drawIndexedIndirect(RID buffer_rid, uint64_t offset,
                                            uint32_t draw_count,
                                            uint32_t stride) {
  (void)buffer_rid;
  (void)offset;
  (void)draw_count;
  (void)stride;
}

// --- Compute ---
void OpenGLCommandList::dispatch(uint32_t group_count_x, uint32_t group_count_y,
                                 uint32_t group_count_z) {
  (void)group_count_x;
  (void)group_count_y;
  (void)group_count_z;
}

// --- Synchronization ---
void OpenGLCommandList::pipelineBarrier(const BarrierInfo &barrier) {
  // OpenGL handles synchronization internally
  (void)barrier;
}

// --- Render Pass Management ---
void OpenGLCommandList::beginRendering(const RenderingInfo &info) {
  (void)info;
}

void OpenGLCommandList::endRendering() {}

// --- Resource Manipulation ---
void OpenGLCommandList::copyBuffer(RID src, RID dst, const BufferCopy &region) {
  (void)src;
  (void)dst;
  (void)region;
}

void OpenGLCommandList::copyBufferToImage(RID src_buffer, RID dst_image,
                                          const BufferImageCopy &region) {
  (void)src_buffer;
  (void)dst_image;
  (void)region;
}

void OpenGLCommandList::clearColorImage(RID image, const float color[4]) {
  glClearColor(color[0], color[1], color[2], color[3]);
  glClear(GL_COLOR_BUFFER_BIT);
  (void)image;
}

} // namespace Render::OpenGL
