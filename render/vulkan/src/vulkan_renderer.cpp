#include "vulkan_renderer.h"

// Includes for the concrete implementation details that have been moved here
#include "render_types.h"
#include "simple_render_system.h" // For the test renderer
#include "vulkan_command_list.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_swap_chain.h"

#include "render_graph_executor.h"
#include "render_scene.h"
#include "render_device.h"

#include <memory>
#include <utility>

namespace Render::Vulkan {

// Constructor now takes ownership of the low-level device
VulkanRenderer::VulkanRenderer(std::unique_ptr<VulkanDevice> device)
    : m_device(std::move(device)) {

  // This logic is temporary for now, to get things compiling.
  // It will be driven by the high-level application setup.
  createSwapChain();

  m_set_layout = DescriptorSetLayout::Builder(*m_device)
                     .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT)
                     .addBinding(1, VK_DESCRIPTOR_TYPE_SAMPLER,
                                 VK_SHADER_STAGE_FRAGMENT_BIT)
                     .addBinding(2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                 VK_SHADER_STAGE_FRAGMENT_BIT, 2)
                     .build();
  m_test_rs = std::make_unique<SimpleRenderSystem>(
      *m_device, m_swap_chain->getRenderPass(),
      m_set_layout->getDescriptorSetLayout());
}

// Explicit destructor implementation in the .cpp file
VulkanRenderer::~VulkanRenderer() {
  // Before destroying the renderer, we must wait for the device to be idle
  // to ensure that no resources are in use.
  if (m_device) {
    vkDeviceWaitIdle(m_device->getDeviceHandle());
  }
}

// This is the main entry point from the Application
void VulkanRenderer::renderFrame(const SceneView &view) {
  // The new, correct sequence of operations, all owned by this class.

  // 1. Acquire an image from the swap chain
  acquireNextImage();
  if (m_swap_chain == nullptr) { // Can happen if swapchain is recreated
    return;
  }

  // 2. Record commands
  VulkanCommandList *cmd =
      m_command_lists[m_swap_chain->getCurrentFrameIndex()].get();
  cmd->begin();

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_swap_chain->getRenderPass();
  renderPassInfo.framebuffer =
      m_swap_chain->getFrameBuffer(m_acquired_image_index);
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_swap_chain->getSwapChainExtent();

  VkClearValue clearColor = {0.1f, 0.1f, 0.1f,
                             1.0f}; // Changed clear color to see changes
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(cmd->getHandle(), &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  m_test_rs->render(cmd->getHandle());


  vkCmdEndRenderPass(cmd->getHandle());
  cmd->end();

  // 3. Submit the command buffer to the GPU
  submitCommands();

  // 4. Present the rendered image to the screen (handled by submitCommands)
  present();
}

void VulkanRenderer::createSwapChain() {
  // This method is now a private part of the VulkanRenderer's setup.
  // TODO: The extent should come from the windowing system.
  m_swap_chain = std::make_unique<VulkanSwapChain>(
      *m_device, VkExtent2D{800, 400}, m_resource_manager);

  // Command lists are tied to the frame lifecycle, so they belong here.
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
  // as per the original design.
}

void VulkanRenderer::acquireNextImage() {
  auto result = m_swap_chain->acquireNextImage(&m_acquired_image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    // For now, we'll just stop rendering. A real app would recreate the
    // swapchain.
    m_swap_chain = nullptr;
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
}

void VulkanRenderer::submitCommands() {
  // This is a simplified version. The real version will get the command list
  // from the RenderGraphExecutor's execution.
  auto vk_command_list =
      m_command_lists[m_swap_chain->getCurrentFrameIndex()].get();
  VkCommandBuffer buffer = vk_command_list->getHandle();

  auto result =
      m_swap_chain->submitCommandBuffers(&buffer, &m_acquired_image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    // For now, we'll just stop rendering. A real app would recreate the
    // swapchain.
    m_swap_chain = nullptr;
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
}
} // namespace Render::Vulkan
