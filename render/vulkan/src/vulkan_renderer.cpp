#include "vulkan_renderer.h"
#include "vulkan_buffer.h"
#include "vulkan_command_list.h"
#include "vulkan_pipeline.h"
#include "vulkan_rhi_device.h"
#include "vulkan_swap_chain.h"

#include "render_graph.h"
#include "render_graph_executor.h"
#include "scene_view.h"

#include <stdexcept>
#include <vector>

namespace Render::Vulkan {

// Constructor now takes ownership of the low-level device
VulkanRenderer::VulkanRenderer(std::unique_ptr<VulkanDevice> device)
    : m_device(std::move(device)) {

  createSwapChain();
  m_rhi_device =
      std::make_unique<VulkanRHIDevice>(*m_device, m_resource_manager);
  m_executor = std::make_unique<RenderGraphExecutor>(m_rhi_device.get());
}

VulkanRenderer::~VulkanRenderer() {
  if (m_device) {
    vkDeviceWaitIdle(m_device->getDeviceHandle());
  }
}

void VulkanRenderer::renderFrame(const Core::SceneView &view) {
  acquireNextImage();
  if (m_swap_chain == nullptr) {
    return;
  }

  VulkanCommandList *cmd =
      m_command_lists[m_swap_chain->getCurrentFrameIndex()].get();
  cmd->begin();

  RID backbuffer_texture_rid =
      m_swap_chain->getTextureRID(m_acquired_image_index);
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
      cmd.draw(3, 1, 0, 0);
    }
  });

  m_executor->execute(graph, backbuffer_texture_rid, *cmd, rect);

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
