#include "opengl_renderer.h"
#include "opengl_command_list.h"
#include "render_data.h"
#include "utils/logger.h"

#include "core/resource_types.h"
#include "core/scene_view.h"
#include "core/uniforms.h"
#include "opengl_buffer_objects.h"
#include "opengl_device.h"
#include "opengl_resource_manager.h"
#include "opengl_shader_program.h"
#include "pipeline_config_registry.h"
#include "render_device.h"
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>
#include <utils/curve_utils.h>

namespace ssme {

//================================================================
// THIS IS LEGACY. WE NEED TO GET RIG OF IT
// по сути это задача экнкодинга команд. она должна выполняться конкретным render pass
// в данном сслучае функция renderGL. это имплементация SimpleRenderPass
void renderGL(CommandList &cmd, const DrawingData &data) {
  // Validate required data
  assert(data.pipeline.isValid() && "DrawingData: pipeline is not valid");
  assert(data.vertex_buffer.isValid() &&
         "DrawingData: vertex_buffer is not valid");
  assert(data.vertex_count > 0 && "DrawingData: vertex_count must be > 0");
  assert(data.getDescriptorSet(0).isValid() &&
         "DrawingData: per-frame descriptor set (0) is required");
  assert(data.getDescriptorSet(1).isValid() &&
         "DrawingData: material descriptor set (1) is required");
  assert(data.getDescriptorSet(2).isValid() &&
         "DrawingData: object descriptor set (2) is required");
  assert(data.hasPushConstants() &&
         "DrawingData: push_constants (model_mat) is required");

  // 0. Bind pipeline
  cmd.setGraphicsPipeline(data.pipeline);

  // 1. Bind vertex buffer
  cmd.setVertexBuffer(0, data.vertex_buffer, 0);

  // 2. Bind descriptor sets
  // Set 0: Per-Frame (camera matrices, projection)
  cmd.setDescriptorSet(0, data.getDescriptorSet(0), data.pipeline);
  // Set 1: Per-Material (material color)
  cmd.setDescriptorSet(1, data.getDescriptorSet(1), data.pipeline);
  // Set 2: Per-Object (normal matrix)
  cmd.setDescriptorSet(2, data.getDescriptorSet(2), data.pipeline);

  // 3. Set push constants (model matrix)
  auto it = data.push_constants.find("model_mat");
  assert(it != data.push_constants.end() &&
         "DrawingData: model_mat push constant is required");
  cmd.setPushConstant(data.pipeline, it->second,
                      static_cast<uint32_t>(ShaderStage::VERTEX));

  // 4. Draw (non-indexed)
  cmd.draw(data.vertex_count, data.instance_count, data.first_vertex, 0);
}

OpenGLRenderer::OpenGLRenderer(Platform *platform)
    : m_platform(platform),
      m_pl_registry(PipelineConfigRegistry(m_backend_type)) {
  /* -------------INIT STATE-------------- */
  m_rhi_device = std::make_unique<ssme::opengl::OpenGLDevice>(
      m_resource_manager, m_pl_registry);
  m_command_list =
      std::make_unique<ssme::opengl::OpenGLCommandList>(m_resource_manager);
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

  // Create per-frame resources (Set 0: camera/projection)
  createPerFrameResources();
}

//================================================================

void OpenGLRenderer::init(ImGuiContext *ctx) {
  /* -------------INIT STATE-------------- */
  m_resource_manager.initialize();
  m_pl_registry.init();
  m_imgui_context = ctx;
  /* -------------INIT UI-------------- */
  const char *glsl_version = "#version 460 core";
  ImGui::SetCurrentContext(m_imgui_context);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

// Explicit destructor in .cpp file allows unique_ptr to see full type
// definitions
OpenGLRenderer::~OpenGLRenderer() {
  m_resource_manager.destroy();
  destroy();
};

void OpenGLRenderer::renderFrame(const SceneView &view,
                                 ImDrawData *ui_draw_data) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Update per-frame uniforms (camera, projection)
  updatePerFrameResources(view);

  // Get per-frame descriptor set RID (Set 0)
  RID per_frame_ds_rid = m_per_frame_resources[0].descriptor_set_rid;

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

  for (const auto &obj : render_objects) {
    // Get material template to access pipeline (shader program)
    auto *material_tpl = m_resource_manager.get_ptr<Material>(obj.material_id);
    if (!material_tpl) {
      Logger::error_log("Opengl Render: not found material");
      continue;
    }
    // // Lookup DrawingPolicy by material type
    // const auto *policy = m_dp_registry.get(material_tpl->name);
    // if (!policy) {
    //   Logger::error_log("Opengl Render: not found drawing policy");
    //   continue;
    // }
    // Get VAO vertex count
    auto vao = m_resource_manager.get_ptr<ssme::opengl::VAO>(obj.geometry_id);
    uint32_t vertex_count = vao ? vao->count() : 0;

    // Setup DrawingData
    DrawingData draw_data;
    draw_data.vertex_buffer = obj.geometry_id;
    draw_data.pipeline = material_tpl->render_data.pipeline;
    draw_data.vertex_count = vertex_count;

    // Descriptor Set 0: Per-Frame (camera/projection)
    draw_data.descriptor_sets[0] = per_frame_ds_rid;
    // Descriptor Set 1: Per-Material (color)
    draw_data.descriptor_sets[1] = material_tpl->render_data.uniforms_ds;
    // Descriptor Set 2: Per-Object (normal matrix) - use object's own
    // descriptor set
    draw_data.descriptor_sets[2] = obj.obj_uniform_ds;

    // Push Constants: model matrix (OpenGL emulates via uniform)
    UniformValue model_matrix_val(obj.model_matrix);
    model_matrix_val.setLabel("model_mat");
    draw_data.push_constants.emplace("model_mat", model_matrix_val);

    // Render using DrawingPolicy
    // заменить на render_pass.encode()
    renderGL(*m_command_list, draw_data);
  }
  if (ui_draw_data) {
    ImGui::SetCurrentContext(m_imgui_context);
    ImGui_ImplOpenGL3_RenderDrawData(ui_draw_data);
  }
}

void OpenGLRenderer::destroy() {
  ImGui::SetCurrentContext(m_imgui_context);
  ImGui_ImplOpenGL3_Shutdown();
}

void OpenGLRenderer::createPerFrameResources() {
  // Skip if already created
  if (m_per_frame_ds_layout) {
    return;
  }
  // Create descriptor set layout for per-frame uniforms (Set 0)
  // Binding 0: GlobalUBO (projectionViewMatrix)
  DescriptorSetLayoutDesc ds_layout_desc;
  ds_layout_desc.bindings.push_back(
      {.binding = 0,
       .type = DescriptorType::UNIFORM_BUFFER,
       .stages = static_cast<uint32_t>(ShaderStage::VERTEX),
       .count = 1});
  m_per_frame_ds_layout =
      m_rhi_device->createDescriptorSetLayout(ds_layout_desc);

  // Create per-frame uniform buffer
  RID frame_uniform_buffer = m_rhi_device->createBuffer(
      BufferDesc{.size = sizeof(Uniforms::FrameUniformsStd140),
                 .usage = static_cast<uint32_t>(BufferUsage::UNIFORM_BUFFER),
                 .is_host_visible = true,
                 .initial_data = nullptr});

  // Create descriptor set
  RID per_frame_ds = m_rhi_device->createDescriptorSet(m_per_frame_ds_layout,
                                                       {frame_uniform_buffer});

  // Store in per-frame resources (OpenGL doesn't have multiple frames in flight
  // like Vulkan)
  m_per_frame_resources.resize(1);
  m_per_frame_resources[0].uniform_buffer = frame_uniform_buffer;
  m_per_frame_resources[0].descriptor_set_rid = per_frame_ds;
}

void OpenGLRenderer::updatePerFrameResources(const SceneView &view) {
  // Get uniform buffer
  auto &frame = m_per_frame_resources[0];
  // Get viewport dimensions for aspect ratio
  GLint viewport_dims[4];
  glGetIntegerv(GL_VIEWPORT, viewport_dims);
  float aspect_ratio = viewport_dims[2] / static_cast<float>(viewport_dims[3]);
  // Calculate view-projection matrix
  // Camera at (0, 0, 5) looking at (0, 0, 0), up is +Y
  glm::mat4 view_mat =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f + view.z), // Camera position
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
  uniforms.camera_position = glm::vec3(0.0f, 0.0f, 5.0f);
  auto packed = Uniforms::FrameUniformsStd140::from(uniforms);
  // Update buffer
  m_rhi_device->updateBufferRaw(frame.uniform_buffer, 0, sizeof(packed),
                                &packed);
}

void OpenGLRenderer::waitIdle() const { glFinish(); }

GpuBackend OpenGLRenderer::getGpuBackend() {
    return m_backend_type;
}

} // namespace ssme
