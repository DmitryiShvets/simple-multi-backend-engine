#include "vulkan_renderer.h"
#include "core/resource_types.h"
#include "core/uniforms.h"
#include "graph/render_graph.h"
#include "resource_manager.h"
#include "scene_view.h"
#include "vulkan_buffer.h"
#include "vulkan_command_list.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_device.h"
#include "vulkan_render_device.h"
#include "vulkan_swap_chain.h"
#include <cassert>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/backends/imgui_impl_vulkan.h>

#include <stdexcept>
#include <vector>

namespace ssme {

void execute(RenderGraph &graph, RID back_buffer, RID depth_buffer,
             CommandList &cmd, const Rect &render_area) {
  graph.compile();

  const auto &passes = graph.getPasses();
  for (const auto &pass : passes) {
    // In a real implementation, RenderingInfo would be constructed based
    // on the pass's `reads` and `writes` dependencies. For now, we
    // hardcode it to render to the backbuffer.
    RenderingInfo rendering_info{};
    rendering_info.render_area = render_area;
    rendering_info.color_attachments.push_back(
        {.texture = back_buffer,
         .load_op = LoadOp::CLEAR,
         .store_op = StoreOp::STORE,
         .clear_value = {0.1f, 0.1f, 0.1f, 1.0f},
         .initial_layout = ImageLayout::UNDEFINED,
         .final_layout = ImageLayout::PRESENT_SRC});
    rendering_info.depth_attachment = {
        .texture = depth_buffer,
        .load_op = LoadOp::CLEAR,
        .store_op = StoreOp::DONT_CARE,
        .clear_value = 1.0f,
    };
    cmd.beginRendering(rendering_info);

    auto &callback = pass->getExecuteCallback();
    if (callback) {
      callback(cmd);
    }

    cmd.endRendering();
  }
}
//================================================================

// Constructor now takes ownership of the low-level device
VulkanRenderer::VulkanRenderer(Platform *platform, ResourceManager *rm)
    : m_platform(platform), m_rm(rm) {
  m_device = std::make_unique<ssme::vulkan::VulkanDevice>(platform);

  /* -------------INIT STATE-------------- */
  createSwapChain();
  m_rhi_device =
      std::make_unique<ssme::vulkan::VulkanRenderDevice>(*m_device, m_storage);
  m_imgui_descriptor_pool =
      ssme::vulkan::VulkanDescriptorPool::Builder(*m_device)
          .addPoolSize(
              vk::DescriptorType::eCombinedImageSampler,
              100) // ImGui mainly needs this type for fonts and textures
          .setPoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
          .setMaxSets(100) // More than enough for ImGui
          .build();
  // createPerFrameResources();
  /* -------------INIT 3D-------------- */
  /* -------------INIT MISC-------------- */
}

void VulkanRenderer::init(ImGuiContext *ctx) {
  /* -------------INIT STATE-------------- */
  m_imgui_context = ctx;
  /* -------------INIT UI-------------- */
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.ApiVersion = VK_API_VERSION_1_4;
  init_info.Instance = m_device->getInstanceHandle();
  init_info.PhysicalDevice = m_device->getPhysicalDeviceHandle();
  init_info.Device = *m_device->getHandle();
  init_info.QueueFamily = m_device->getPhysicalQueueFamilies().graphics_family;
  init_info.Queue = *m_device->getGraphicsQueue();
  // init_info.PipelineCache = g_PipelineCache;
  init_info.DescriptorPool = m_imgui_descriptor_pool->getDescriptorPool();
  init_info.MinImageCount = 2;
  init_info.ImageCount = m_swap_chain->getImageCount();
  init_info.Allocator = nullptr;
  // --- Dynamic Rendering Setup ---
  init_info.UseDynamicRendering = true;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount =
      1;
  auto color_format = (VkFormat)m_swap_chain->getSwapChainImageFormat();
  auto depth_format = (VkFormat)m_swap_chain->findDepthFormat();
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

VulkanRenderer::~VulkanRenderer() { destroy(); }

void VulkanRenderer::renderFrame(SceneView &view, ImDrawData *ui_draw_data) {
  acquireNextImage(); // wait for fences and retrives new image

  // Update per-frame uniforms (camera, projection)
  updatePerFrameResources(view);

  ssme::vulkan::VulkanCommandList *cmd =
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
  auto &frame_resources = m_frame_data;
  RID per_frame_ds_rid =
      m_frame_data->uniform_ds[frame_index]->getDescriptorSetId();

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
    for (auto &renderable : view.opaque_objects) {
      renderable.setDescriptor(0, per_frame_ds_rid);
      draw(cmd, renderable);
    }

    if (ui_draw_data) {
      ImGui::SetCurrentContext(m_imgui_context);
      ImGui_ImplVulkan_RenderDrawData(
          ui_draw_data,
          static_cast<ssme::vulkan::VulkanCommandList &>(cmd).getHandle());
    }
  });

  // TODO: SETUP NEW UI PASS
  // if (ui_draw_data) {
  //   auto &ui_pass = graph.addPass("UI Pass");
  //   ui_pass.setExecuteCallback([&](CommandList &cmd) {
  //     // Call ImGui rendering inside this pass callback
  //     // We need a specific VkCommandBuffer, so we cast it
  //     ImGui::SetCurrentContext(m_imgui_context);
  //     ImGui_ImplVulkan_RenderDrawData(
  //         ui_draw_data, static_cast<VulkanCommandList &>(cmd).getHandle());
  //   });
  // }

  execute(graph, backbuffer_texture_rid, backbuffer_depth_texture_rid, *cmd,
          rect);

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
  m_device->getHandle().waitIdle();
  ImGui::SetCurrentContext(m_imgui_context);
  ImGui_ImplVulkan_Shutdown();
}

void VulkanRenderer::createSwapChain() {
  m_swap_chain = std::make_unique<ssme::vulkan::VulkanSwapChain>(
      *m_device, vk::Extent2D{800, 400}, m_storage);

  m_command_lists.clear();
  m_command_lists.reserve(MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    m_command_lists.push_back(std::make_unique<ssme::vulkan::VulkanCommandList>(
        *m_device, m_storage));
  }
}

void VulkanRenderer::setFrameResources(std::shared_ptr<FrameData> data) {
  m_frame_data = data;
}

void VulkanRenderer::updatePerFrameResources(const SceneView &view) {
  // Get uniform buffer
  auto frame_index = m_swap_chain->getCurrentFrameIndex();
  auto &ubo = m_frame_data->uniform_buffer[frame_index];
  // Calculate view-projection matrix
  // Camera at (0, 0, 5) looking at (0, 0, 0), up is +Y
  glm::mat4 view_mat = glm::lookAt(
      glm::vec3(0.0f + view.x, 0.0f, 5.0f + view.z), // Camera position
      glm::vec3(0.0f, 0.0f, 0.0f),                   // Look at target
      glm::vec3(0.0f, 1.0f, 0.0f)                    // Up direction
  );
  // Vulkan uses Y-down clip space, so we need to flip Y axis
  glm::mat4 proj_mat = glm::perspective(
      glm::radians(45.0f), m_swap_chain->extentAspectRatio(), 0.1f, 100.0f);
  proj_mat[1][1] *= -1.0f; // Flip Y for Vulkan

  Uniforms::FrameUniforms uniforms{};
  uniforms.view_projection = proj_mat * view_mat;
  uniforms.light_position = glm::vec3(0.0f, 0.0f, 1.f);
  uniforms.Kd =
      glm::vec3(1.0f, 1.0f, 1.0f); // Diffuse coefficient (white surface)
  uniforms.Ld = glm::vec3(1.0f, 1.0f, 1.0f); // Light intensity (white light)
  uniforms.camera_position = glm::vec3(0.0f, 5.0f, 5.0f);
  auto packed = Uniforms::FrameUniformsStd140::from(uniforms);
  ubo->update(&packed, sizeof(packed));
}

void VulkanRenderer::present() {
  // This is intentionally left empty.
  // The presentation is handled by `VulkanSwapChain::submitCommandBuffers`
}

void VulkanRenderer::acquireNextImage() {
  vk::Result result = m_swap_chain->acquireNextImage(&m_acquired_image_index);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    m_swap_chain = nullptr;
    return;
  } else if (result != vk::Result::eSuccess &&
             result != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("Failed to acquire swap chain image!");
  }
}

void VulkanRenderer::submitCommands() {
  auto vk_command_list =
      m_command_lists[m_swap_chain->getCurrentFrameIndex()].get();
  vk::CommandBuffer buffer = vk_command_list->getHandle();

  vk::Result result =
      m_swap_chain->submitCommandBuffers(&buffer, &m_acquired_image_index);

  if (result == vk::Result::eErrorOutOfDateKHR ||
      result == vk::Result::eSuboptimalKHR) {
    m_swap_chain = nullptr;
  } else if (result != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to present swap chain image!");
  }
}

void VulkanRenderer::waitIdle() const { m_device->getHandle().waitIdle(); }

GpuBackend VulkanRenderer::getGpuBackend() { return m_backend_type; }

} // namespace ssme
