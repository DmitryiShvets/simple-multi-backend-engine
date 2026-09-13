#include "opengl_shader_module.h"

#include "utils/common_utils.h"
#include <iostream>
#include <stdexcept>
#include <vector>

namespace ssme::opengl {
OpenGLShaderModule::OpenGLShaderModule(const GLenum type,
                                       const std::string &shader_path) {
  auto code = CUtils::readFileChar(shader_path);
  code.push_back('\0');
  m_shader_module = createShader(code.data(), type);
}
OpenGLShaderModule::OpenGLShaderModule(const GLenum type,
                                       const std::vector<char> &code) {
  // code.push_back('\0');
  m_shader_module = createShader(code.data(), type);
}

OpenGLShaderModule::~OpenGLShaderModule() { glDeleteShader(m_shader_module); }

GLuint OpenGLShaderModule::createShader(const char *source, const GLenum type) {
  GLuint shader_module = glCreateShader(type);
  glShaderSource(shader_module, 1, &source, nullptr);
  glCompileShader(shader_module);
  // Check for compile time errors
  GLint success;
  glGetShaderiv(shader_module, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(shader_module, 512, nullptr, infoLog);
    GLType errType(type);
    std::cout << "ERROR::SHADER::" << errType << "::COMPILATION_FAILED\n"
              << infoLog << std::endl;
    throw std::runtime_error(infoLog);
  }
  return shader_module;
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

} // namespace ssme::opengl
