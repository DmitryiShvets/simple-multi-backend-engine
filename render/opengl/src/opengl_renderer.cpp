#include "opengl_renderer.h"
#include "opengl_buffer_objects.h"
#include <curve_utils.h>

RenderObject::RenderObject(const std::string &uuid, const std::string &shaderId,
                           const glm::vec3 &color,
                           const std::vector<glm::vec3> &coordsArray)
    : uuid(uuid), shaderId(shaderId), color(color) {
  VBOLayout gradient_layout;
  gradient_layout.addLayoutElement(2, GL_FLOAT, GL_FALSE);

  auto vertices = std::vector<float>();
  for (auto &x : coordsArray) {
    vertices.push_back(x.x);
    vertices.push_back(x.y);
  }
  // auto vertices = CurveUtils::createLineGeometry(coordsArray, 0.005);
  VAO objVAO;
  VBO objVBO;

  uint64_t count = vertices.size() / 2;
  objVAO.bind();
  objVBO.init(vertices.data(), 2 * count * sizeof(GLfloat));
  objVAO.addBuffer(objVBO, gradient_layout, count);
  objVBO.unbind();
  objVAO.unbind();

  vao = std::move(objVAO);
}

void OpenGLRenderer::draw(const VAO &vao, const EBO &ebo) {
  vao.bind();
  ebo.bind();
  glDrawElements(GL_TRIANGLES, ebo.count(), GL_UNSIGNED_INT, 0);
  vao.unbind();
  ebo.unbind();
}

void OpenGLRenderer::draw(const VAO &vao) {
  vao.bind();
  glDrawArrays(GL_TRIANGLES, 0, vao.count());
  vao.unbind();
}

void OpenGLRenderer::draw(VAO *vao) {
  vao->bind();
  glDrawArrays(GL_TRIANGLES, 0, vao->count());
  vao->unbind();
}

void OpenGLRenderer::draw(VAO *vao, EBO *ebo) {
  vao->bind();
  ebo->bind();
  glDrawElements(GL_TRIANGLES, ebo->count(), GL_UNSIGNED_INT, 0);
  vao->unbind();
  ebo->unbind();
}

void OpenGLRenderer::setClearColor(float r, float g, float b, float a) {
  glClearColor(r, g, b, a);
}

void OpenGLRenderer::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::initialize(int width, int height) {
  m_resource_manager = std::make_unique<OpenglResourceManager>();
  m_resource_manager->initialize();
}

void OpenGLRenderer::drawBundle(const std::string &shader, void *bundle) {
  auto t = (Render::Renderable *)bundle;
  ShaderProgram *mProgram = &m_resource_manager->getProgram(shader);
  mProgram->use();
  mProgram->setUniform("customColor", glm::vec4(t->color, 1.0));
  auto vao = &m_resource_manager->getVAO(t->mesh_id);
  OpenGLRenderer::draw(vao);
  mProgram->unbind();
}

void OpenGLRenderer::destroy() { m_resource_manager->destroy(); }

void OpenGLRenderer::frame(float delta_time) {}

void OpenGLRenderer::resize(int width, int height) {}

void OpenGLRenderer::setViewPort(int x, int y, int width, int height) {
  glViewport(x, y, width, height);
}
