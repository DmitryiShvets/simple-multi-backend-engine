#include "opengl_resource_manager.h"
#include "opengl_buffer_objects.h"
#include "opengl_shader_program.h"
#include "opengl_descriptor_set.h"

#include "common_utils.h"

#include <iostream>
#include <memory>

namespace Render::OpenGL {

OpenglResourceManager::OpenglResourceManager() {
  std::cout << "Constructor OpenglResourceManager (" << this << ") called "
            << std::endl;
}

OpenglResourceManager::~OpenglResourceManager() {
  std::cout << "Destructor OpenglResourceManager (" << this << ") called "
            << std::endl;
}

void OpenglResourceManager::initialize() {

  // auto p1 = std::make_unique<ShaderProgram>(
  //     CUtils::readFile("res/shaders/v_default.glsl"),
  //     CUtils::readFile("res/shaders/f_default.glsl"));
  // auto p2 = std::make_unique<ShaderProgram>(
  //     CUtils::readFile("res/shaders/v_default.glsl"),
  //     CUtils::readFile("res/shaders/f_custom_color.glsl"));
  // add<ShaderProgram>(std::move(p1));
  // add<ShaderProgram>(std::move(p2));

  // TODO: CREATING BUFFER HELPER
  // VBOLayout menuVBOLayout;
  // menuVBOLayout.addLayoutElement(2, GL_FLOAT, GL_FALSE);

  // const GLfloat vertex[] = {// x(s)  y(t)
  //                           0.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f};
  // VAO baseVAO;
  // VBO baseVBO;

  // baseVAO.bind();
  // baseVBO.init(vertex, 2 * 3 * sizeof(GLfloat));
  // baseVAO.addBuffer(baseVBO, menuVBOLayout, 3);
  // baseVBO.unbind();
  // baseVAO.unbind();

  // m_vao.emplace("default", std::move(baseVAO));
  // for test
  // auto coords = CurveUtils::createCircleCoords(0, 0, 0.7, 32);
}

void OpenglResourceManager::destroy() {}

// void OpenglResourceManager::addRenderObject(
//     const std::string &uuid, const std::vector<glm::vec3> &coordsArray) {
//   VBOLayout layout;
//   layout.addLayoutElement(2, GL_FLOAT, GL_FALSE);

//   auto vertices = std::vector<float>();
//   for (auto &x : coordsArray) {
//     vertices.push_back(x.x);
//     vertices.push_back(x.y);
//   }
//   // auto vertices = CurveUtils::createLineGeometry(coordsArray, 0.005);
//   VAO objVAO;
//   VBO objVBO;

//   uint64_t count = vertices.size() / 2;
//   objVAO.bind();
//   objVBO.init(vertices.data(), 2 * count * sizeof(GLfloat));
//   objVAO.addBuffer(objVBO, layout, count);
//   objVBO.unbind();
//   objVAO.unbind();

//   m_vao.emplace(uuid, std::move(objVAO));
// }
} // namespace Render::OpenGL
