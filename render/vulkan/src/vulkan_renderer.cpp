#include "vulkan_renderer.h"
#include "logger.h"
#include "pipeline_config_registry.h"
#include "resource_types.h"
#include "vertex.h"
#include "vulkan_buffer.h"
#include "vulkan_command_list.h"
#include "vulkan_pipeline.h"
#include "vulkan_rhi_device.h"
#include "vulkan_swap_chain.h"

#include "render_graph.h"
#include "render_graph_executor.h"
#include "scene_view.h"
#include "uniforms.h"

#include <cassert>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui_impl_vulkan.h"

#include <stdexcept>
#include <vector>

namespace Render::Vulkan {

// Constructor now takes ownership of the low-level device
VulkanRenderer::VulkanRenderer(std::unique_ptr<VulkanDevice> device)
    : m_device(std::move(device)),
      m_pl_registry(PipelineConfigRegistry(m_backend_type)) {
  /* -------------INIT STATE-------------- */
  createSwapChain();
  m_rhi_device = std::make_unique<VulkanRHIDevice>(
      *m_device, m_resource_manager, m_pl_registry);
  m_imgui_descriptor_pool =
      DescriptorPool::Builder(*m_device)
          .addPoolSize(
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              100) // ImGui mainly needs this type for fonts and textures
          .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
          .setMaxSets(100) // More than enough for ImGui
          .build();
  createPerFrameResources();
  /* -------------INIT 3D-------------- */
  /* -------------INIT MISC-------------- */
  m_executor = std::make_unique<RenderGraphExecutor>(m_rhi_device.get());
}

void VulkanRenderer::init(ImGuiContext *ctx) {
  /* -------------INIT STATE-------------- */
  m_imgui_context = ctx;
  /* -------------INIT DRAWING POLICIES-------------- */
  m_pl_registry.init();
  m_dp_registry.init();
  /* -------------INIT UI-------------- */
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.ApiVersion = VK_API_VERSION_1_4;
  init_info.Instance = m_device->getInstanceHandle();
  init_info.PhysicalDevice = m_device->getPhysicalDeviceHandle();
  init_info.Device = m_device->getDeviceHandle();
  init_info.QueueFamily = m_device->findPhysicalQueueFamilies().graphics_family;
  init_info.Queue = m_device->getGraphicsQueue();
  // init_info.PipelineCache = g_PipelineCache;
  init_info.DescriptorPool = m_imgui_descriptor_pool->get_descriptor_pool();
  init_info.MinImageCount = 2;
  init_info.ImageCount = m_swap_chain->getImageCount();
  init_info.Allocator = nullptr;
  // --- Dynamic Rendering Setup ---
  init_info.UseDynamicRendering = true;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount =
      1;
  auto color_format = m_swap_chain->getSwapChainImageFormat();
  auto depth_format = m_swap_chain->findDepthFormat();
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo
      .pColorAttachmentFormats = &color_format;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat =
      depth_format;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo
      .stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

  // init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
  // init_info.PipelineInfoMain.Subpass = 0;
  init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  // init_info.CheckVkResultFn = check_vk_result;
  ImGui::SetCurrentContext(m_imgui_context);
  ImGui_ImplVulkan_Init(&init_info);
}

VulkanRenderer::~VulkanRenderer() {}

void VulkanRenderer::renderFrame(const Core::SceneView &view,
                                 ImDrawData *ui_draw_data) {
  acquireNextImage();

  // Update per-frame uniforms (camera, projection)
  updatePerFrameResources(view);

  VulkanCommandList *cmd =
      m_command_lists[m_swap_chain->getCurrentFrameIndex()].get();
  cmd->begin();

  RID backbuffer_texture_rid =
      m_swap_chain->getTextureRID(m_acquired_image_index);
  RID backbuffer_depth_texture_rid =
      m_swap_chain->getDepthTextureRID(m_acquired_image_index);
  auto extent = m_swap_chain->getSwapChainExtent();
  auto rect = Rect{
      .x = 0,
      .y = 0,
      .width = extent.width,
      .height = extent.height,
  };
  // Barrier 1: Undefined -> Color Attachment
  BarrierInfo to_render_barrier;
  to_render_barrier.image_barriers.push_back({
      .image = backbuffer_texture_rid,
      .old_layout = ImageLayout::UNDEFINED,
      .new_layout = ImageLayout::COLOR_ATTACHMENT,
  });
  cmd->pipelineBarrier(to_render_barrier);

  // Build and execute the render graph for this frame
  RenderGraph graph;
  auto &pass = graph.addPass("Opaque Pass");

  // Get per-frame descriptor set RID
  auto frame_index = m_swap_chain->getCurrentFrameIndex();
  auto &frame_resources = m_per_frame_resources[frame_index];
  RID per_frame_ds_rid = frame_resources.descriptor_set_rid;

  // The actual drawing logic is now in this callback
  pass.setExecuteCallback([&](CommandList &cmd) {
    cmd.setViewport({
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    });
    cmd.setScissor(rect);

    // Get DrawingPolicy registry
    for (const auto &renderable : view.opaque_objects) {
      // Get pipeline from material template
      auto *material_tpl =
          m_resource_manager.get_ptr<Material>(renderable.material_id);
      if (!material_tpl) {
        continue; // Skip if material template not found
      }
      // Lookup DrawingPolicy by material type
      const auto *policy = m_dp_registry.get(material_tpl->name);
      if (!policy) {
        continue; // Skip if policy not found
      }

      // Get vertex count from geometry buffer
      // Use vertex stride from buffer descriptor if available
      auto *geom_buffer =
          m_resource_manager.get_ptr<VulkanDataBuffer>(renderable.geometry_id);
      uint32_t vertex_count = 3; // Default fallback
      if (geom_buffer) {
        // TODO: Get vertex stride from geometry descriptor
        // For now, assume VertexN (24 bytes) for ads material, Vertex (12 bytes) for default
        uint32_t stride = (material_tpl->name == "default") ? 12 : 24;
        vertex_count = static_cast<uint32_t>(geom_buffer->getBufferSize() / stride);
      }

      // Render using DrawingPolicy
      DrawingData draw_data;
      draw_data.vertex_buffer = renderable.geometry_id;
      draw_data.pipeline = material_tpl->render_data.pipeline;
      draw_data.vertex_count = vertex_count;

      // Descriptor Set 0: Per-Frame (camera/projection)
      draw_data.descriptor_sets[0] = per_frame_ds_rid;
      // Descriptor Set 1: Per-Material (color)
      draw_data.descriptor_sets[1] = material_tpl->render_data.uniforms_ds;
      // Descriptor Set 2: Per-Object (model matrix) - use object's own descriptor set
      // For materials without object uniforms (e.g., default), this will be INVALID
      draw_data.descriptor_sets[2] = renderable.obj_uniform_ds;

      // Push Constants: model matrix (Vulkan uses push constants, not uniforms)
      draw_data.push_constants.emplace("model_mat", renderable.model_matrix);

      policy->render(cmd, draw_data);
    }

    if (ui_draw_data) {
      ImGui::SetCurrentContext(m_imgui_context);
      ImGui_ImplVulkan_RenderDrawData(
          ui_draw_data, static_cast<VulkanCommandList &>(cmd).getHandle());
    }
  });

  // TODO: SETUP NEW UI PASS
  // if (ui_draw_data) {
  //   auto &ui_pass = graph.addPass("UI Pass");
  //   ui_pass.setExecuteCallback([&](CommandList &cmd) {
  //     // Вызываем отрисовку ImGui внутри коллбека этого пасса
  //     // Нам нужен конкретный VkCommandBuffer, поэтому делаем каст
  //     ImGui::SetCurrentContext(m_imgui_context);
  //     ImGui_ImplVulkan_RenderDrawData(
  //         ui_draw_data, static_cast<VulkanCommandList &>(cmd).getHandle());
  //   });
  // }

  m_executor->execute(graph, backbuffer_texture_rid,
                      backbuffer_depth_texture_rid, *cmd, rect);

  // Barrier 2: Color Attachment -> Present
  BarrierInfo to_present_barrier;
  to_present_barrier.image_barriers.push_back({
      .image = backbuffer_texture_rid,
      .old_layout = ImageLayout::COLOR_ATTACHMENT,
      .new_layout = ImageLayout::PRESENT_SRC,
  });
  cmd->pipelineBarrier(to_present_barrier);

  cmd->end();

  submitCommands();
  present();
}

void VulkanRenderer::destroy() {
  vkDeviceWaitIdle(m_device->getDeviceHandle());
  ImGui::SetCurrentContext(m_imgui_context);
  ImGui_ImplVulkan_Shutdown();
}

void VulkanRenderer::createSwapChain() {
  m_swap_chain = std::make_unique<VulkanSwapChain>(
      *m_device, VkExtent2D{800, 400}, m_resource_manager);

  m_command_lists.clear();
  m_command_lists.reserve(m_swap_chain->getImageCount());
  for (size_t i = 0; i < m_swap_chain->getImageCount(); i++) {
    m_command_lists.push_back(
        std::make_unique<VulkanCommandList>(*m_device, m_resource_manager));
  }
}

void VulkanRenderer::createPerFrameResources() {
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
  // Create per-frame resources
  auto frame_count = m_swap_chain->getImageCount();
  m_per_frame_resources.resize(frame_count);
  for (uint32_t i = 0; i < frame_count; i++) {
    // Create uniform buffer for FrameUniforms
    RID frame_uniform_buffer = m_rhi_device->createBuffer(
        BufferDesc{.size = sizeof(Core::Uniforms::FrameUniformsStd140),
                   .usage = static_cast<uint32_t>(BufferUsage::UNIFORM_BUFFER),
                   .is_host_visible = true,
                   .initial_data = nullptr});
    // Create descriptor set
    RID per_frame_ds = m_rhi_device->createDescriptorSet(
        m_per_frame_ds_layout, {frame_uniform_buffer});

    m_per_frame_resources[i].uniform_buffer = frame_uniform_buffer;
    m_per_frame_resources[i].descriptor_set_rid = per_frame_ds;
  }
}

void VulkanRenderer::updatePerFrameResources(const Core::SceneView &view) {
  // Get uniform buffer
  auto frame_index = m_swap_chain->getCurrentFrameIndex();
  auto &frame = m_per_frame_resources[frame_index];
  // Calculate view-projection matrix
  // Camera at (0, 0, 5) looking at (0, 0, 0), up is +Y
  glm::mat4 view_mat =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f + view.z), // Camera position
                  glm::vec3(0.0f, 0.0f, 0.0f),          // Look at target
                  glm::vec3(0.0f, 1.0f, 0.0f)           // Up direction
      );
  // Vulkan uses Y-down clip space, so we need to flip Y axis
  glm::mat4 proj_mat = glm::perspective(
      glm::radians(45.0f), m_swap_chain->extentAspectRatio(), 0.1f, 100.0f);
  proj_mat[1][1] *= -1.0f; // Flip Y for Vulkan

  Core::Uniforms::FrameUniforms uniforms{};
  uniforms.view_projection = proj_mat * view_mat;
  uniforms.light_position = glm::vec3(0.0f, 0.0f, 1.f);
  uniforms.Kd =
      glm::vec3(1.0f, 1.0f, 1.0f); // Diffuse coefficient (white surface)
  uniforms.Ld = glm::vec3(1.0f, 1.0f, 1.0f); // Light intensity (white light)
  uniforms.camera_position = glm::vec3(0.0f, 5.0f, 5.0f);
  auto packed = Core::Uniforms::FrameUniformsStd140::from(uniforms);
  // Update buffer
  m_rhi_device->updateBufferRaw(frame.uniform_buffer, 0, sizeof(packed),
                                &packed);
}

void VulkanRenderer::present() {
  // This is intentionally left empty.
  // The presentation is handled by `VulkanSwapChain::submitCommandBuffers`
}

void VulkanRenderer::acquireNextImage() {
  auto result = m_swap_chain->acquireNextImage(&m_acquired_image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    m_swap_chain = nullptr;
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
}

void VulkanRenderer::submitCommands() {
  auto vk_command_list =
      m_command_lists[m_swap_chain->getCurrentFrameIndex()].get();
  VkCommandBuffer buffer = vk_command_list->getHandle();

  auto result =
      m_swap_chain->submitCommandBuffers(&buffer, &m_acquired_image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    m_swap_chain = nullptr;
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
}
} // namespace Render::Vulkan
