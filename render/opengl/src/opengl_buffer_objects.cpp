#include "opengl_buffer_objects.h"

namespace Render::OpenGL {

// *************** UniformBuffer *********************

UniformBuffer::UniformBuffer(size_t size, const void* data) 
    : m_ubo(0), m_size(size) {
    glCreateBuffers(1, &m_ubo);
    glNamedBufferData(m_ubo, static_cast<GLsizeiptr>(size), data, GL_DYNAMIC_DRAW);
}

UniformBuffer::~UniformBuffer() {
    if (m_ubo != 0) {
        glDeleteBuffers(1, &m_ubo);
    }
}

void UniformBuffer::update(size_t offset, size_t size, const void* data) {
    glNamedBufferSubData(m_ubo, static_cast<GLintptr>(offset), 
                         static_cast<GLsizeiptr>(size), data);
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept 
    : m_ubo(other.m_ubo), m_size(other.m_size) {
    other.m_ubo = 0;
    other.m_size = 0;
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
    if (this != &other) {
        if (m_ubo != 0) {
            glDeleteBuffers(1, &m_ubo);
        }
        m_ubo = other.m_ubo;
        m_size = other.m_size;
        other.m_ubo = 0;
        other.m_size = 0;
    }
    return *this;
}

// *************** VBO *********************

VBO::VBO() : mVBO(0) {}

VBO::~VBO() { glDeleteBuffers(1, &mVBO); }

VBO::VBO(VBO &&vbo) noexcept {
  mVBO = vbo.mVBO;

  vbo.mVBO = 0;
}

VBO &VBO::operator=(VBO &&vbo) noexcept {
  if (this != &vbo) {
    glDeleteBuffers(1, &mVBO);
    mVBO = vbo.mVBO;

    vbo.mVBO = 0;
  }
  return *this;
}

void VBO::init(const void *data, const unsigned int size) {
  glGenBuffers(1, &mVBO);
  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void VBO::update(const void *data, const unsigned int size) const {
  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

void VBO::bind() const { glBindBuffer(GL_ARRAY_BUFFER, mVBO); }

void VBO::unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

EBO::EBO() : mEBO(0), mCount(0) {}

EBO::~EBO() { glDeleteBuffers(1, &mEBO); }

EBO::EBO(EBO &&ebo) noexcept {
  mEBO = ebo.mEBO;
  mCount = ebo.mCount;

  ebo.mEBO = 0;
  ebo.mCount = 0;
}

EBO &EBO::operator=(EBO &&ebo) noexcept {
  if (this != &ebo) {
    glDeleteBuffers(1, &mEBO);
    mEBO = ebo.mEBO;
    mCount = ebo.mCount;

    ebo.mEBO = 0;
    ebo.mCount = 0;
  }
  return *this;
}

void EBO::init(const void *data, const unsigned int count) {
  mCount = count;
  glGenBuffers(1, &mEBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data,
               GL_STATIC_DRAW);
}

void EBO::bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO); }

void EBO::unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

unsigned int EBO::count() const { return mCount; }

VBOLayout::VBOLayout() : mStride(0) {}

void VBOLayout::addLayoutElement(GLint count, GLenum type,
                                 GLboolean normalized) {
  mVecLayoutElements.push_back(
      {count, type, normalized, count * sizeof(GLfloat)});
  mStride += mVecLayoutElements.back().size;
}

VBOLayout::~VBOLayout() { mVecLayoutElements.clear(); }

unsigned int VBOLayout::getStride() const { return mStride; }

const std::vector<VBOLayoutElements> &VBOLayout::getLayoutElements() const {
  return mVecLayoutElements;
}

VAO::VAO() : mVAO(0) { glGenVertexArrays(1, &mVAO); }

void VAO::bind() const { glBindVertexArray(mVAO); }

void VAO::unbind() const { glBindVertexArray(0); }

void VAO::addBuffer(const VBO &buffer, const VBOLayout &layout,
                    const unsigned int countVertex) {
  bind();
  buffer.bind();
  GLbyte *offset = nullptr;
  const auto &elements = layout.getLayoutElements();
  for (int i = 0; i < elements.size(); ++i) {
    const auto &currentLayout = elements[i];
    auto k = mBuffersCount + i;
    glEnableVertexAttribArray(k);
    glVertexAttribPointer(k, currentLayout.count, currentLayout.type,
                          currentLayout.normalized, layout.getStride(), offset);
    offset += currentLayout.size;
  }
  mBuffersCount += elements.size();
  if (countVertex != 0)
    mVertexCount = countVertex;
}

unsigned int VAO::count() const { return mVertexCount; }

VAO::~VAO() { glDeleteVertexArrays(1, &mVAO); }

VAO::VAO(VAO &&vao) noexcept {
  mVAO = vao.mVAO;
  mVertexCount = vao.mVertexCount;

  vao.mVAO = 0;
  vao.mVertexCount = 0;
}

VAO &VAO::operator=(VAO &&vao) noexcept {
  if (this != &vao) {
    glDeleteVertexArrays(1, &mVAO);
    mVAO = vao.mVAO;
    mVertexCount = vao.mVertexCount;

    vao.mVAO = 0;
    vao.mVertexCount = 0;
  }
  return *this;
}
} // namespace Render::OpenGL
