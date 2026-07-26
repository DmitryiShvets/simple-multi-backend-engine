#include "opengl_renderer.h"
#include "core/resource_types.h"
#include "scene_view.h"
#include "core/uniforms.h"
#include "opengl_buffer_objects.h"
#include "opengl_command_list.h"
#include "opengl_render_device.h"
#include "opengl_shader_program.h"
#include "render_device.h"
#include "render_item.h"
#include "resource_manager.h"
#include "utils/logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>
#include <utils/curve_utils.h>

namespace ssme {



OpenGLRenderer::OpenGLRenderer(Platform *platform, ResourceManager *rm)
    : m_platform(platform), m_rm(rm) {
  /* -------------INIT STATE-------------- */
  m_rhi_device = std::make_unique<ssme::opengl::OpenGLRenderDevice>(m_storage);
  m_command_list = std::make_unique<ssme::opengl::OpenGLCommandList>(m_storage);
  // m_scene_renderer = std::make_unique<SceneRenderer>();
  // m_executor = std::make_unique<RenderGraphExecutor>(m_rhi_device.get());
  /* -------------SETUP 3D-------------- */
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  /* -------------SETUP MISC-------------- */
  // glClearColor(175.0f / 255.0f, 218.0f / 255.0f, 252.0f / 255.0f, 1.0f);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glEnable(GL_FRAMEBUFFER_SRGB);
}

//================================================================

void OpenGLRenderer::init(ImGuiContext *ctx) {
  /* -------------INIT STATE-------------- */
  m_imgui_context = ctx;
  /* -------------INIT UI-------------- */
  const char *glsl_version = "#version 460 core";
  ImGui::SetCurrentContext(m_imgui_context);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

// Explicit destructor in .cpp file allows unique_ptr to see full type
// definitions
OpenGLRenderer::~OpenGLRenderer() { destroy(); };

void OpenGLRenderer::renderFrame(SceneView &view, ImDrawData *ui_draw_data) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Update per-frame uniforms (camera, projection)
  updatePerFrameResources(view);

  // Get per-frame descriptor set RID (Set 0)
  RID per_frame_ds_rid = m_frame_data->uniform_ds[0]->getDescriptorSetId();

  // Setup viewport and scissor
  int viewport_width = 800, viewport_height = 600; // TODO: Get from window
  m_command_list->setViewport({
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(viewport_width),
      .height = static_cast<float>(viewport_height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  });
  m_command_list->setScissor({
      .x = 0,
      .y = 0,
      .width = static_cast<uint32_t>(viewport_width),
      .height = static_cast<uint32_t>(viewport_height),
  });

  auto render_objects = view.opaque_objects;

  for (auto &obj : render_objects) {
    obj.setDescriptor(0, per_frame_ds_rid);
    draw(*m_command_list, obj);
  }

  if (ui_draw_data) {
    ImGui::SetCurrentContext(m_imgui_context);
    ImGui_ImplOpenGL3_RenderDrawData(ui_draw_data);
  }
}

void OpenGLRenderer::destroy() {
    if(m_imgui_context) {
        ImGui::SetCurrentContext(m_imgui_context);
        ImGui_ImplOpenGL3_Shutdown();
    }
}

void OpenGLRenderer::setFrameResources(std::shared_ptr<FrameData> data) {
  m_frame_data = data;
}

void OpenGLRenderer::updatePerFrameResources(const SceneView &view) {
  // Get uniform buffer
  auto &ubo = m_frame_data->uniform_buffer[0];
  // Get viewport dimensions for aspect ratio
  GLint viewport_dims[4];
  glGetIntegerv(GL_VIEWPORT, viewport_dims);
  float aspect_ratio = viewport_dims[2] / static_cast<float>(viewport_dims[3]);
  // Calculate view-projection matrix
  // Camera at (0, 0, 5) looking at (0, 0, 0), up is +Y
  glm::mat4 view_mat =
      glm::lookAt(glm::vec3(0.0f + view.x, 0.0f, 5.0f + view.z), // Camera position
                  glm::vec3(0.0f, 0.0f, 0.0f),          // Look at target
                  glm::vec3(0.0f, 1.0f, 0.0f)           // Up direction
      );
  glm::mat4 proj_mat =
      glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 100.0f);

  Uniforms::FrameUniforms uniforms{};
  uniforms.view_projection = proj_mat * view_mat;
  uniforms.light_position = glm::vec3(0.0f, 0.0f, 1.0f);
  uniforms.Kd =
      glm::vec3(1.0f, 1.0f, 1.0f); // Diffuse coefficient (white surface)
  uniforms.Ld = glm::vec3(1.0f, 1.0f, 1.0f); // Light intensity (white light)
  uniforms.camera_position = glm::vec3(0.0f, 5.0f, 5.0f);
  auto packed = Uniforms::FrameUniformsStd140::from(uniforms);
  // Update buffer
  m_rhi_device->updateBuffer(ubo->getUbo(), packed);
}

void OpenGLRenderer::waitIdle() const { glFinish(); }

GpuBackend OpenGLRenderer::getGpuBackend() { return m_backend_type; }

} // namespace ssme
