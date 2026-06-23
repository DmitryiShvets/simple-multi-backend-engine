#include "d3d12_renderer.h"
#include "com_exception.h"
#include "core/render_types.h"
#include "core/resource_types.h"
#include "core/uniforms.h"
#include "d3dx12.h"
#include "dx12_command_list.h"
#include "dx12_device.h"
#include "dx12_render_device.h"
#include "dx12_swap_chain.h"
#include "render_device.h"
#include "render_item.h"
#include "resource_manager.h"
#include "scene_view.h"
#include "utils/logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <utils/curve_utils.h>
namespace ssme {


Dx12Renderer::Dx12Renderer(Platform *platform, ResourceManager *rm)
    : m_platform(platform), m_rm(rm) {
  /* -------------INIT STATE-------------- */
  m_device = std::make_unique<ssme::d3d12::Dx12Device>(platform);
  m_rhi_device =
      std::make_unique<ssme::d3d12::Dx12RenderDevice>(*m_device, m_storage);
  m_swap_chain = std::make_unique<ssme::d3d12::Dx12SwapChain>(
      *m_device, Extent2D{800, 400}, m_storage, platform);

  m_command_lists.clear();
  m_command_lists.reserve(MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    m_command_lists.push_back(
        std::make_unique<ssme::d3d12::Dx12CommandList>(*m_device, m_storage));
  }
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
  // Phase 1: Acquire (CPU ждёт, если GPU занят)
  m_swap_chain->acquireNextImage();
  // Get per-frame descriptor set RID (Set 0)
  RID per_frame_ds_rid = m_frame_data->uniform_ds[0]->getDescriptorSetId();
  // auto render_objects = view.opaque_objects;

  // Update uniforms
  // updatePerFrameResources(view);

  auto frame_index = m_swap_chain->getCurrentFrameIndex();
  auto *cmd = m_command_lists[frame_index].get();
  auto native = cmd->getCommandListHandle();
  RID backbuffer_texture_rid =
      m_swap_chain->getTextureRID(frame_index);
  cmd->begin();

  // Barrier: PRESENT → RENDER_TARGET
  BarrierInfo to_render_barrier;
  to_render_barrier.image_barriers.push_back({
      .image = backbuffer_texture_rid,
      .old_layout = ImageLayout::PRESENT_SRC,
      .new_layout = ImageLayout::COLOR_ATTACHMENT,
  });
  cmd->pipelineBarrier(to_render_barrier);
  // Clear
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swap_chain->getRtvHandle(frame_index);
  const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
  native->ClearRenderTargetView(rtvHandle, clearColor, 0,
                                                    nullptr);
  // if (ui_draw_data) {
  //   // ImGui::SetCurrentContext(m_imgui_context);
  //   // ImGui_ImplOpenGL3_RenderDrawData(ui_draw_data);
  // }

  // Barrier: RENDER_TARGET → PRESENT
  BarrierInfo to_present_barrier;
  to_present_barrier.image_barriers.push_back({
      .image = backbuffer_texture_rid,
      .old_layout = ImageLayout::COLOR_ATTACHMENT,
      .new_layout = ImageLayout::PRESENT_SRC,
  });
  cmd->pipelineBarrier(to_present_barrier);

  cmd->end();
  // Phase 2: Submit + Signal + Present (CPU не ждёт)
  m_swap_chain->submitCommandBuffers(native);
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
  glm::mat4 view_mat = glm::lookAt(
      glm::vec3(0.0f + view.x, 0.0f, 5.0f + view.z), // Camera position
      glm::vec3(0.0f, 0.0f, 0.0f),                   // Look at target
      glm::vec3(0.0f, 1.0f, 0.0f)                    // Up direction
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

void Dx12Renderer::waitIdle() const {}

GpuBackend Dx12Renderer::getGpuBackend() { return m_backend_type; }

} // namespace ssme
