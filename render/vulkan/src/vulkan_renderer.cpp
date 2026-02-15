#include "vulkan_renderer.h"
#include "vulkan_buffer.h"
#include "vulkan_command_list.h"
#include "vulkan_pipeline.h"
#include "vulkan_rhi_device.h"
#include "vulkan_swap_chain.h"

#include "render_graph.h"
#include "render_graph_executor.h"
#include "scene_view.h"

#include "imgui_impl_vulkan.h"

#include <stdexcept>
#include <vector>

namespace Render::Vulkan {

// Constructor now takes ownership of the low-level device
VulkanRenderer::VulkanRenderer(std::unique_ptr<VulkanDevice> device)
    : m_device(std::move(device)) {
  /* -------------INIT STATE-------------- */
  createSwapChain();
  m_rhi_device =
      std::make_unique<VulkanRHIDevice>(*m_device, m_resource_manager);
  m_imgui_descriptor_pool =
      DescriptorPool::Builder(*m_device)
          .addPoolSize(
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              100) // ImGui в основном нужен этот тип для шрифтов и текстур
          .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
          .setMaxSets(100) // Более чем достаточно для ImGui
          .build();
  /* -------------INIT 3D-------------- */
  /* -------------INIT MISC-------------- */
  m_executor = std::make_unique<RenderGraphExecutor>(m_rhi_device.get());
}

void VulkanRenderer::init(ImGuiContext *ctx) {
  /* -------------INIT STATE-------------- */
  m_imgui_context = ctx;
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
  // --- Настройка для Dynamic Rendering ---
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
  if (m_swap_chain == nullptr) {
    return;
  }

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

    for (const auto &renderable : view.opaque_objects) {
      cmd.setGraphicsPipeline(renderable.material_id);
      cmd.setVertexBuffer(0, renderable.geometry_id, 0);
      // TODO: Get vertex count from geometry resource
      cmd.draw(1500, 1, 0, 0);
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
