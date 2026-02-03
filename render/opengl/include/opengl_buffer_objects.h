#pragma once
#include <vector>
#include <glad/gl.h>

class VBO {
public:
    VBO();

    void init(const void* data, const unsigned int size);

    void update(const void* data, const unsigned int size) const;

    void bind() const;

    void unbind() const;

    ~VBO();

    VBO(const VBO&) = delete;

    VBO& operator=(const VBO&) = delete;

    VBO(VBO&& vbo) noexcept;

    VBO& operator=(VBO&& vbo) noexcept;

private:
    GLuint mVBO;
};


class EBO {
public:
    EBO();

    void init(const void* data, const unsigned int count);

    void bind() const;

    void unbind() const;

    unsigned int count() const;

    ~EBO();

    EBO(const EBO&) = delete;

    EBO& operator=(const EBO&) = delete;

    EBO(EBO&& ebo) noexcept;

    EBO& operator=(EBO&& ebo) noexcept;

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

    const std::vector<VBOLayoutElements>& getLayoutElements() const;

private:
    unsigned int mStride;
    std::vector<VBOLayoutElements> mVecLayoutElements;
};

class VAO {
public:
    VAO();

    void bind() const;

    void unbind() const;

    void addBuffer(const VBO& buffer, const VBOLayout& layout, const unsigned  int countVertex = 0);

    unsigned int count() const;

    ~VAO();

    VAO(const VAO&) = delete;

    VAO& operator=(const VAO&) = delete;

    VAO(VAO&& vao) noexcept;

    VAO& operator=(VAO&& vao) noexcept;

private:
    GLuint mVAO;
    unsigned int mBuffersCount = 0;
    unsigned int mVertexCount = 0;
};
