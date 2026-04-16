#pragma once

#include <glad/gl.h>
#include <string>

namespace ssme::opengl {
class GLType {

public:
  GLuint type;

  explicit GLType(GLuint thatType);

  friend std::ostream &operator<<(std::ostream &lhs, const GLType e);
};

class OpenGLShaderModule {
public:
  OpenGLShaderModule(const GLenum type, const std::string &shader_path);
  ~OpenGLShaderModule();

  GLuint getHandle() const { return m_shader_module; }

private:
  GLuint createShader(const char *source, const GLenum type);

  GLuint m_shader_module;
};

} // namespace ssme::opengl
