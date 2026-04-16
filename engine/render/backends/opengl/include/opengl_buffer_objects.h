#pragma once
#include <glad/gl.h>
#include <vector>
#include <cstddef>
#include "core/vertex_layout.h"

namespace ssme::opengl {

// Uniform Buffer Object - stores uniform data in GPU memory
class UniformBuffer {
public:
    UniformBuffer(size_t size, const void* data);
    ~UniformBuffer();

    // Update data in buffer
    void update(size_t offset, size_t size, const void* data);

    // Map buffer to CPU memory for read/write
    void* map();
    void unmap();

    GLuint getHandle() const { return m_ubo; }
    size_t getSize() const { return m_size; }

    // Delete copy
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    // Move
    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(UniformBuffer&& other) noexcept;

private:
    GLuint m_ubo;
    size_t m_size;
};

class VBO {
public:
  VBO();

  void init(const void *data, const unsigned int size);

  void update(const void *data, const unsigned int size) const;

  void bind() const;

  void unbind() const;

  ~VBO();

  VBO(const VBO &) = delete;

  VBO &operator=(const VBO &) = delete;

  VBO(VBO &&vbo) noexcept;

  VBO &operator=(VBO &&vbo) noexcept;

private:
  GLuint mVBO;
};

class EBO {
public:
  EBO();

  void init(const void *data, const unsigned int count);

  void bind() const;

  void unbind() const;

  unsigned int count() const;

  ~EBO();

  EBO(const EBO &) = delete;

  EBO &operator=(const EBO &) = delete;

  EBO(EBO &&ebo) noexcept;

  EBO &operator=(EBO &&ebo) noexcept;

private:
  GLuint mEBO;

  unsigned int mCount;
};

struct VBOLayoutElements {
  GLint count;
  GLenum type;
  GLboolean normalized;
  unsigned long long size;
};

class VBOLayout {
public:
  VBOLayout();

  ~VBOLayout();

  void addLayoutElement(GLint count, GLenum type, GLboolean normalized);

  unsigned int getStride() const;

  const std::vector<VBOLayoutElements> &getLayoutElements() const;

private:
  unsigned int mStride;
  std::vector<VBOLayoutElements> mVecLayoutElements;
};

class VAO {
public:
  VAO();

  void bind() const;

  void unbind() const;

  void addBuffer(const VBO &buffer, const VBOLayout &layout,
                 const unsigned int countVertex = 0);

  void addBuffer(const VBO &buffer, const VertexLayout &layout,
                 const unsigned int countVertex = 0);

  unsigned int count() const;

  ~VAO();

  VAO(const VAO &) = delete;

  VAO &operator=(const VAO &) = delete;

  VAO(VAO &&vao) noexcept;

  VAO &operator=(VAO &&vao) noexcept;

private:
  GLuint mVAO;
  unsigned int mBuffersCount = 0;
  unsigned int mVertexCount = 0;
};

} // namespace ssme::opengl
