#include "opengl_renderer.h"

#include "render_graph_executor.h"
#include "render_scene.h"
#include "render_device.h"
#include "opengl_resource_manager.h"
#include "scene_view.h"
#include "opengl_buffer_objects.h"
#include <curve_utils.h>
namespace Render::OpenGL {

OpenGLRenderer::OpenGLRenderer() {
  // For example:
  // m_rhi_device = std::make_unique<OpenGLDevice>();
  // m_scene_renderer = std::make_unique<SceneRenderer>();
  // m_executor = std::make_unique<RenderGraphExecutor>(m_rhi_device.get());
  m_resource_manager = std::make_unique<OpenglResourceManager>();
  m_resource_manager->initialize();
  glClearColor(175.0f / 255.0f, 218.0f / 255.0f, 252.0f / 255.0f,
                            1.0f);
}

// Explicit destructor in .cpp file allows unique_ptr to see full type
// definitions
OpenGLRenderer::~OpenGLRenderer() {
    m_resource_manager->destroy();
};

void OpenGLRenderer::renderFrame(const SceneView &view) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto render_objects = view.renderables;
    for (auto& obj : render_objects) {
        ShaderProgram *mProgram = &m_resource_manager->getProgram(obj.shader_id);
        mProgram->use();
        mProgram->setUniform("customColor", glm::vec4(obj.color, 1.0));
        auto vao = &m_resource_manager->getVAO(obj.mesh_id);
        // OpenGLRenderer::draw(vao);
        vao->bind();
        glDrawArrays(GL_TRIANGLES, 0, vao->count());
        vao->unbind();
        mProgram->unbind();
    }

}

} // namespace Render::OpenGL
