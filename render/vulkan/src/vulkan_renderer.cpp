#include "vulkan_renderer.h"

// Includes for the concrete implementation details that have been moved here
#include "render_types.h"
#include "simple_render_system.h" // For the test renderer
#include "vulkan_buffer.h"        // new
#include "vulkan_command_list.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_swap_chain.h"
#include "vulkan_texture.h" // new

#include "render_device.h"
#include "render_graph_executor.h"
#include "render_scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace Render::Vulkan {

struct GlobalUbo {
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::vec3 lightDir = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
};

// Constructor now takes ownership of the low-level device
VulkanRenderer::VulkanRenderer(std::unique_ptr<VulkanDevice> device)
    : m_device(std::move(device)) {

  // This logic is temporary for now, to get things compiling.
  // It will be driven by the high-level application setup.
  createSwapChain();

  m_set_layout = DescriptorSetLayout::Builder(*m_device).build();
  m_test_rs = std::make_unique<SimpleRenderSystem>(
      *m_device, m_swap_chain->getRenderPass(),
      m_set_layout->getDescriptorSetLayout());

  // ====================================================================
  // DEMONSTRATION of VulkaDataBuffer and VulkanTexture usage
  // ====================================================================

  // 1. Create a uniform buffer using your new wrapper class
  const int FRAMES_IN_FLIGHT = m_swap_chain->getImageCount();
  const std::vector<Vertex> vertices = {
      {{0.0f, -1.f, 0.f}, {1.0f, 0.0f, 0.0f}}, // вершина 1, красная
      {{1.f, 1.0f, 0.f}, {0.0f, 1.0f, 0.0f}},  // вершина 2, зеленая
      {{-1.f, 1.f, 0.f}, {0.0f, 0.0f, 1.0f}}  // вершина 3, синяя
  };
  m_vertex_buffer = std::make_unique<VulkanDataBuffer>(
      *m_device, sizeof(vertices[0]) * vertices.size(), 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  m_vertex_buffer->map();
  m_vertex_buffer->writeToBuffer((void *)vertices.data());

  // 2. Create a texture using your new wrapper class
  m_texture =
      std::make_unique<VulkanTexture>(*m_device, "res/textures/texture.jpg");

  // 3. Create a descriptor pool
  m_global_pool =
      DescriptorPool::Builder(*m_device).setMaxSets(FRAMES_IN_FLIGHT).build();

  // 4. Use the DescriptorWriter to bind the buffer and texture to a descriptor
  // set
  auto bufferInfo = m_vertex_buffer->descriptorInfo();
  m_global_descriptor_set =
      DescriptorWriter(*m_set_layout, *m_global_pool).build();
  // ====================================================================
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

  VkBuffer buffers[] = {m_vertex_buffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd->getHandle(), 0, 1, buffers, offsets);

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
