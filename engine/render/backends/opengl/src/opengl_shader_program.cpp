#include "opengl_shader_program.h"
#include "core/resource_types.h"
#include "opengl_shader_module.h"
#include "core/glsl_layout.h"
#include "core/uniform_value.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace ssme::opengl {

ShaderProgram::ShaderProgram(const OpenGLShaderModule &vert_shader,
                             const OpenGLShaderModule &frag_shader) {

  GLuint hVertexSh = vert_shader.getHandle();
  GLuint hFragmentSh = frag_shader.getHandle();

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
}

bool ShaderProgram::isCompiled() const { return compiled; }

void ShaderProgram::use() { glUseProgram(hProgram); }

void ShaderProgram::unbind() { glUseProgram(0); }

// ShaderProgram::~ShaderProgram() {
//   glDeleteProgram(hProgram);
//   hProgram = 0;
// }
ShaderProgram::~ShaderProgram() {
  if (m_push_ubo != 0)
    glDeleteBuffers(1, &m_push_ubo);
  glDeleteProgram(hProgram);
  hProgram = 0;
}

// ShaderProgram &ShaderProgram::operator=(ShaderProgram &&program) noexcept {
//   if (this != &program) {
//     glDeleteProgram(hProgram);
//     hProgram = program.hProgram;
//     compiled = program.compiled;

//     program.hProgram = 0;
//     program.compiled = false;
//   }
//   return *this;
// }

ShaderProgram &ShaderProgram::operator=(ShaderProgram &&program) noexcept {
  if (this != &program) {
    if (m_push_ubo != 0)
      glDeleteBuffers(1, &m_push_ubo);
    glDeleteProgram(hProgram);
    hProgram = program.hProgram;
    compiled = program.compiled;
    m_push_ubo = program.m_push_ubo;
    m_push_size = program.m_push_size;

    program.hProgram = 0;
    program.compiled = false;
    program.m_push_ubo = 0;
    program.m_push_size = 0;
  }
  return *this;
}


// ShaderProgram::ShaderProgram(ShaderProgram &&program) noexcept {
//   hProgram = program.hProgram;
//   compiled = program.compiled;

//   program.hProgram = 0;
//   program.compiled = false;
// }

ShaderProgram::ShaderProgram(ShaderProgram &&program) noexcept {
  hProgram = program.hProgram;
  compiled = program.compiled;
  m_push_ubo = program.m_push_ubo;
  m_push_size = program.m_push_size;

  program.hProgram = 0;
  program.compiled = false;
  program.m_push_ubo = 0;
  program.m_push_size = 0;
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

void ShaderProgram::setPushConstant(const UniformValue &value,
                                    uint32_t offset) {
  const size_t needed = offset + value.size();
  if (m_push_ubo == 0)
    glGenBuffers(1, &m_push_ubo);

  if (needed > m_push_size) {
    m_push_size = static_cast<uint32_t>(needed);
    glBindBuffer(GL_UNIFORM_BUFFER, m_push_ubo);
    glBufferData(GL_UNIFORM_BUFFER, m_push_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  glBindBuffer(GL_UNIFORM_BUFFER, m_push_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(offset),
                  static_cast<GLsizeiptr>(value.size()), value.data());
  glBindBufferBase(GL_UNIFORM_BUFFER, kOpenglPushBinding, m_push_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

} // namespace ssme::opengl
