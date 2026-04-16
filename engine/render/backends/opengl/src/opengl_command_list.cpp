#include "opengl_command_list.h"
#include "opengl_buffer_objects.h"
#include "opengl_descriptor_set.h"
#include "opengl_gpu_storage.h"
#include "opengl_shader_program.h"

namespace ssme::opengl {

OpenGLCommandList::OpenGLCommandList(OpenGLGpuStorageMT &storage)
    : m_storage(storage) {}

// --- Pipeline State ---
void OpenGLCommandList::setGraphicsPipeline(RID pipeline_rid) {
  // Bind shader program immediately
  auto *program = m_storage.get<ShaderProgram>(pipeline_rid);
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
  auto *vao = m_storage.get<VAO>(buffer_rid);
  if (vao) {
    vao->bind();
  }
}

void OpenGLCommandList::setIndexBuffer(RID buffer_rid, uint64_t offset,
                                       IndexType type) {
  auto *ebo = m_storage.get<EBO>(buffer_rid);
  if (ebo) {
    ebo->bind();
  }
}

void OpenGLCommandList::setDescriptorSet(uint32_t set_index, RID set_rid,
                                         RID pipeline_rid) {
  (void)set_index;
  (void)pipeline_rid;

  // Get DescriptorSet and bind all resources
  auto *desc_set = m_storage.get<OpenGLDescriptorSet>(set_rid);
  if (desc_set) {
    desc_set->bind(set_index); // Calls glBindBufferBase for each binding
  }
}

void OpenGLCommandList::setPushConstant(RID pipeline_rid,
                                        const UniformValue &value,
                                        ShaderStageFlags stages,
                                        uint32_t offset) {
  auto *program = m_storage.get<ShaderProgram>(pipeline_rid);
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
  glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
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

} // namespace ssme::opengl
