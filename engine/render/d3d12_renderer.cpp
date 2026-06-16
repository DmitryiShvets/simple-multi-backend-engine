#include "d3d12_renderer.h"
#include "core/resource_types.h"
#include "dx12_command_list.h"
#include "dx12_render_device.h"
#include "scene_view.h"
#include "core/uniforms.h"
#include "render_device.h"
#include "render_item.h"
#include "resource_manager.h"
#include "utils/logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <utils/curve_utils.h>

namespace ssme {



Dx12Renderer::Dx12Renderer(Platform *platform, ResourceManager *rm)
    : m_platform(platform), m_rm(rm) {
  /* -------------INIT STATE-------------- */
  m_rhi_device = std::make_unique<ssme::d3d12::Dx12RenderDevice>(m_storage);
  m_command_list = std::make_unique<ssme::d3d12::Dx12CommandList>(m_storage);
  /* -------------SETUP 3D-------------- */

  /* -------------SETUP MISC-------------- */

}

//================================================================

void Dx12Renderer::init(ImGuiContext *ctx) {
  /* -------------INIT STATE-------------- */
  // m_imgui_context = ctx;
  /* -------------INIT UI-------------- */
  // const char *glsl_version = "#version 460 core";
  // ImGui::SetCurrentContext(m_imgui_context);
  // ImGui_ImplOpenGL3_Init(glsl_version);
}

// Explicit destructor in .cpp file allows unique_ptr to see full type
// definitions
Dx12Renderer::~Dx12Renderer() { destroy(); };

void Dx12Renderer::renderFrame(SceneView &view, ImDrawData *ui_draw_data) {
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
    // ImGui::SetCurrentContext(m_imgui_context);
    // ImGui_ImplOpenGL3_RenderDrawData(ui_draw_data);
  }
}

void Dx12Renderer::destroy() {
  // ImGui::SetCurrentContext(m_imgui_context);
  // ImGui_ImplOpenGL3_Shutdown();
}

void Dx12Renderer::setFrameResources(std::shared_ptr<FrameData> data) {
  m_frame_data = data;
}

void Dx12Renderer::updatePerFrameResources(const SceneView &view) {
  // Get uniform buffer
  auto &ubo = m_frame_data->uniform_buffer[0];
  // Get viewport dimensions for aspect ratio
  float aspect_ratio = 1;
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
  ubo->update(&packed, sizeof(packed));
}

void Dx12Renderer::waitIdle() const { }

GpuBackend Dx12Renderer::getGpuBackend() { return m_backend_type; }

} // namespace ssme
