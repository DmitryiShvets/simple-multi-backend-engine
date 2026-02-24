#include "opengl_shader_program.h"
#include "resource_types.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace Render::OpenGL {

ShaderProgram::ShaderProgram(const char *vertexShader,
                             const char *fragmentShader) {
  // std::cout << "Constructor ShaderProgram (" << this << ") called " <<
  // std::endl;
  GLuint hVertexSh;
  if (!createShader(vertexShader, GL_VERTEX_SHADER, hVertexSh))
    return;

  GLuint hFragmentSh;
  if (!createShader(fragmentShader, GL_FRAGMENT_SHADER, hFragmentSh)) {
    glDeleteShader(hVertexSh);
    return;
  }
  hProgram = glCreateProgram();
  glAttachShader(hProgram, hVertexSh);
  glAttachShader(hProgram, hFragmentSh);
  glLinkProgram(hProgram);
  // Check for linking errors
  GLint success;
  glGetProgramiv(hProgram, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetProgramInfoLog(hProgram, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  } else {
    compiled = true;
  }
  glDeleteShader(hVertexSh);
  glDeleteShader(hFragmentSh);
}

bool ShaderProgram::createShader(const char *source, const GLenum type,
                                 GLuint &hShader) {
  hShader = glCreateShader(type);
  glShaderSource(hShader, 1, &source, nullptr);
  glCompileShader(hShader);
  // Check for compile time errors
  GLint success;
  glGetShaderiv(hShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(hShader, 512, nullptr, infoLog);
    GLType errType(type);
    std::cout << "ERROR::SHADER::" << errType << "::COMPILATION_FAILED\n"
              << infoLog << std::endl;
    return false;
  }
  return true;
}

bool ShaderProgram::isCompiled() const { return compiled; }

void ShaderProgram::use() { glUseProgram(hProgram); }

void ShaderProgram::unbind() { glUseProgram(0); }

ShaderProgram::~ShaderProgram() {
  // std::cout << "Destructor ShaderProgram (" << this << ") called " <<
  // std::endl;
  glDeleteProgram(hProgram);
  hProgram = 0;
}

ShaderProgram::ShaderProgram(const std::string &vertexShader,
                             const std::string &fragmentShader)
    : ShaderProgram(vertexShader.c_str(), fragmentShader.c_str()) {}

ShaderProgram &ShaderProgram::operator=(ShaderProgram &&program) noexcept {
  // std::cout << "Assignment-Move ShaderProgram (" << this << ") called " <<
  // std::endl;
  if (this != &program) {
    glDeleteProgram(hProgram);
    hProgram = program.hProgram;
    compiled = program.compiled;

    program.hProgram = 0;
    program.compiled = false;
  }
  return *this;
}

ShaderProgram::ShaderProgram(ShaderProgram &&program) noexcept {
  // std::cout << "Constructor-Move ShaderProgram (" << this << ") called " <<
  // std::endl;
  hProgram = program.hProgram;
  compiled = program.compiled;

  program.hProgram = 0;
  program.compiled = false;
}

GLuint &ShaderProgram::getUintProgram() { return hProgram; }

void ShaderProgram::setUniform(const UniformValue &value) {
  if (!value.hasLabel()) {
    throw std::runtime_error("Opengl UNIFORM must have label!");
  }
  value.visit([this, &value](const auto &data) {
    using T = std::decay_t<decltype(data)>;

    if constexpr (std::is_same_v<T, glm::mat4>) {
      setUniform(value.getLabel(), data);
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
      setUniform(value.getLabel(), data);
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
      setUniform(value.getLabel(), data);
    } else if constexpr (std::is_same_v<T, int>) {
      setUniform(value.getLabel(), data);
    } else if constexpr (std::is_same_v<T, float>) {
      setUniform(value.getLabel(), data);
    }
  });
}

void ShaderProgram::setUniform(const std::string &uniformName,
                               const glm::mat4 &matrixValue) {
  glUniformMatrix4fv(glGetUniformLocation(hProgram, uniformName.c_str()), 1,
                     GL_FALSE, glm::value_ptr(matrixValue));
}

void ShaderProgram::setUniform(const std::string &uniformName,
                               const glm::vec4 &vec4Value) {
  glUniform4f(glGetUniformLocation(hProgram, uniformName.c_str()), vec4Value.x,
              vec4Value.y, vec4Value.z, vec4Value.w);
}

void ShaderProgram::setUniform(const std::string &uniformName,
                               const glm::vec3 &vec3Value) {
  glUniform3f(glGetUniformLocation(hProgram, uniformName.c_str()), vec3Value.x,
              vec3Value.y, vec3Value.z);
}

void ShaderProgram::setUniform(const std::string &uniformName, float value) {
  glUniform1f(glGetUniformLocation(hProgram, uniformName.c_str()), value);
}

void ShaderProgram::setUniform(const std::string &uniformName, int value) {
  glUniform1i(glGetUniformLocation(hProgram, uniformName.c_str()), value);
}

std::ostream &operator<<(std::ostream &lhs, const GLType e) {
  switch (e.type) {
  case GL_VERTEX_SHADER:
    lhs << "GL_VERTEX_SHADER";
    break;
  case GL_FRAGMENT_SHADER:
    lhs << "GL_FRAGMENT_SHADER";
    break;
  }
  return lhs;
}

GLType::GLType(GLuint thatType) { type = thatType; }
} // namespace Render::OpenGL
