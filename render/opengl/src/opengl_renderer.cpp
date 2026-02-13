#include "opengl_renderer.h"

#include "opengl_buffer_objects.h"
#include "opengl_device.h"
#include "opengl_resource_manager.h"
#include "opengl_shader_program.h"
#include "render_device.h"
#include "render_graph_executor.h"
#include "render_scene.h"
#include "scene_view.h"
#include <curve_utils.h>
namespace Render::OpenGL {

OpenGLRenderer::OpenGLRenderer() {
  // For example:
  m_rhi_device = std::make_unique<OpenGLDevice>(m_resource_manager);
  // m_scene_renderer = std::make_unique<SceneRenderer>();
  // m_executor = std::make_unique<RenderGraphExecutor>(m_rhi_device.get());
  m_resource_manager.initialize();
  glClearColor(175.0f / 255.0f, 218.0f / 255.0f, 252.0f / 255.0f, 1.0f);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
}

// Explicit destructor in .cpp file allows unique_ptr to see full type
// definitions
OpenGLRenderer::~OpenGLRenderer() { m_resource_manager.destroy(); };

void OpenGLRenderer::renderFrame(const Core::SceneView &view) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  auto render_objects = view.opaque_objects;
  for (auto &obj : render_objects) {
    ShaderProgram *mProgram =
        m_resource_manager.get_ptr<ShaderProgram>(obj.material_id);
    mProgram->use();
    //mProgram->setUniform("customColor", glm::vec4(1.f, 0.f, 0.f
     //   , 1.0));
    auto vao = m_resource_manager.get_ptr<VAO>(obj.geometry_id);
    // OpenGLRenderer::draw(vao);
    vao->bind();
    glDrawArrays(GL_TRIANGLES, 0, vao->count());
    vao->unbind();
    mProgram->unbind();
  }
}

} // namespace Render::OpenGL
