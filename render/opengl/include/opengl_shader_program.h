#pragma once
#include "resource_types.h"
#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <string>

namespace Core {
class UniformValue;
}

namespace Render::OpenGL {
class GLType {

public:
  GLuint type;

  explicit GLType(GLuint thatType);

  friend std::ostream &operator<<(std::ostream &lhs, const GLType e);
};

class ShaderProgram {

public:
  ShaderProgram(const char *vertexShader, const char *fragmentShader);

  ShaderProgram(const std::string &vertexShader,
                const std::string &fragmentShader);

  bool isCompiled() const;

  void use();
  void unbind();

  GLuint &getUintProgram();

  void setUniform(const UniformValue& value);
  void setUniform(const std::string &uniformName, const glm::mat4 &matrixValue);
  void setUniform(const std::string &uniformName, int value);
  void setUniform(const std::string &uniformName, float value);
  void setUniform(const std::string &uniformName, const glm::vec4 &vec4Value);
  void setUniform(const std::string &uniformName, const glm::vec3 &vec3Value);

  ~ShaderProgram();

  ShaderProgram(const ShaderProgram &) = delete;
  ShaderProgram &operator=(const ShaderProgram &) = delete;
  ShaderProgram(ShaderProgram &&program) noexcept;
  ShaderProgram &operator=(ShaderProgram &&program) noexcept;

private:
  bool createShader(const char *source, const GLenum type, GLuint &hShader);

  bool compiled = false;

  GLuint hProgram = 0;
};
} // namespace Render::OpenGL
