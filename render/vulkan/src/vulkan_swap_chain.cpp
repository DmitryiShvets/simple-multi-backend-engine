#include "vulkan_swap_chain.h"
#include "vulkan_texture.h"
#include "vulkan_types.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace Render::Vulkan {

VulkanSwapChain::VulkanSwapChain(VulkanDevice &deviceRef, VkExtent2D extent,
                                 VulkanResourceManager &resourceManager)
    : m_device{deviceRef}, m_window_extent{extent},
      m_resource_manager(resourceManager) {
  init();
}

VulkanSwapChain::VulkanSwapChain(VulkanDevice &deviceRef,
                                 VkExtent2D windowExtent,
                                 std::shared_ptr<VulkanSwapChain> previous,
                                 VulkanResourceManager &resourceManager)
    : m_device{deviceRef}, m_window_extent{windowExtent},
      m_old_swapchain{previous}, m_resource_manager(resourceManager) {
  init();
  // The old swapchain is no longer needed after its resources are reused.
  m_old_swapchain = nullptr;
}

VulkanSwapChain::~VulkanSwapChain() {
  // Destroy all created objects in reverse order of creation.

  // 1. Swapchain object itself. Image views are now owned by VulkanTexture
  // objects.
  if (m_swap_chain != nullptr) {
    vkDestroySwapchainKHR(m_device.getDeviceHandle(), m_swap_chain, nullptr);
    m_swap_chain = nullptr;
  }

  // 2. Framebuffers
  for (auto framebuffer : m_swap_chain_framebuffers) {
    vkDestroyFramebuffer(m_device.getDeviceHandle(), framebuffer, nullptr);
  }

  // 3. Render Pass
  vkDestroyRenderPass(m_device.getDeviceHandle(), m_render_pass, nullptr);

  // 4. Synchronization objects
  auto FRAMES_IN_FLIGHT = getImageCount();
  for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(m_device.getDeviceHandle(),
                       m_render_finished_semaphores[i], nullptr);
    vkDestroySemaphore(m_device.getDeviceHandle(),
                       m_image_available_semaphores[i], nullptr);
    vkDestroyFence(m_device.getDeviceHandle(), m_in_flight_fences[i], nullptr);
  }
}

void VulkanSwapChain::init() {
  // This function orchestrates the creation of all swapchain-related objects.
  createSwapChain();
  createTextureWrappers();
  createRenderPass();
  createFramebuffers();
  createSyncObjects();
}

VkResult VulkanSwapChain::acquireNextImage(uint32_t *imageIndex) {
  // Wait until the fence associated with the current frame is signaled.
  // This means the GPU has finished rendering the frame from a previous loop.
  // This prevents the CPU from getting more than FRAMES_IN_FLIGHT frames
  // ahead of the GPU.
  vkWaitForFences(m_device.getDeviceHandle(), 1,
                  &m_in_flight_fences[m_current_frame], VK_TRUE,
                  std::numeric_limits<uint64_t>::max());

  // Acquire the next available image from the swapchain.
  // The `imageAvailableSemaphores` will be signaled when the image is ready to
  // be rendered to.
  VkResult result = vkAcquireNextImageKHR(
      m_device.getDeviceHandle(), m_swap_chain,
      std::numeric_limits<uint64_t>::max(),
      m_image_available_semaphores[m_current_frame], // must be a not signaled
                                                     // semaphore
      VK_NULL_HANDLE, imageIndex);

  return result;
}

VkResult VulkanSwapChain::submitCommandBuffers(const VkCommandBuffer *buffers,
                                               uint32_t *imageIndex) {
  // If the image we are submitting to is still in flight (being used by a
  // previous frame), we must wait for its fence to be signaled.
  if (m_images_in_flight[*imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(m_device.getDeviceHandle(), 1,
                    &m_images_in_flight[*imageIndex], VK_TRUE, UINT64_MAX);
  }
  // Mark the image as now being in use by the current frame.
  m_images_in_flight[*imageIndex] = m_in_flight_fences[m_current_frame];

  // --- Configure the submission ---
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Specify which semaphores to wait on before execution begins.
  // We wait on the `imageAvailableSemaphores`, and we wait until the pipeline
  // stage that writes to the color attachment is reached.
  VkSemaphore wait_semaphores[] = {
      m_image_available_semaphores[m_current_frame]};
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;

  // Specify the command buffers to execute.
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = buffers;

  // Specify which semaphores to signal once the command buffer has finished
  // execution.
  VkSemaphore signal_semaphores[] = {
      m_render_finished_semaphores[m_current_frame]};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  // Reset the fence to an unsignaled state before using it.
  vkResetFences(m_device.getDeviceHandle(), 1,
                &m_in_flight_fences[m_current_frame]);

  // Submit the command buffer to the graphics queue.
  // The `inFlightFences` will be signaled when the command buffer has finished
  // execution.
  if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submit_info,
                    m_in_flight_fences[m_current_frame]) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  // --- Configure the presentation ---
  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  // Wait on the `renderFinishedSemaphores` before presentation can happen.
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;

  // Specify the swap chain to present to.
  VkSwapchainKHR swap_chains[] = {m_swap_chain};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swap_chains;
  present_info.pImageIndices = imageIndex;

  // Submit the request to present the image to the screen.
  auto result = vkQueuePresentKHR(m_device.getPresentQueue(), &present_info);

  // Advance to the next frame index.
  auto FRAMES_IN_FLIGHT = getImageCount();
  m_current_frame = (m_current_frame + 1) % FRAMES_IN_FLIGHT;

  return result;
}

void VulkanSwapChain::createSwapChain() {
  // This commented-out block contains the logic for creating the VkSwapchainKHR
  // object.

  // 1. Get all the details about what the physical device and surface support.
  SwapChainSupportDetails swap_chain_support = m_device.getSwapChainSupport();

  // 2. Choose the best settings from the available options.
  VkSurfaceFormatKHR surface_format =
      chooseSwapSurfaceFormat(swap_chain_support.formats);
  VkPresentModeKHR present_mode =
      chooseSwapPresentMode(swap_chain_support.present_modes);
  VkExtent2D extent = chooseSwapExtent(swap_chain_support.capabilities);

  // 3. Decide on the number of images in the swap chain. It's good practice to
  // request at least one more than the minimum to avoid waiting on the driver.
  uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
  if (swap_chain_support.capabilities.maxImageCount > 0 &&
      image_count > swap_chain_support.capabilities.maxImageCount) {
    image_count = swap_chain_support.capabilities.maxImageCount;
  }

  // 4. Fill out the main creation structure.
  VkSwapchainCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = m_device.getSurface();

  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
      VK_IMAGE_USAGE_TRANSFER_DST_BIT; // We'll be drawing to these images.

  // 5. Handle how images are used across different queue families.
  QueueFamilyIndices indices = m_device.findPhysicalQueueFamilies();
  uint32_t queue_family_indices[] = {indices.graphics_family,
                                     indices.present_family};

  if (indices.graphics_family != indices.present_family) {
    // If graphics and present queues are different, we use CONCURRENT mode.
    // This is less performant but easier to manage.
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    // If they are the same, we use EXCLUSIVE mode, which is more performant.
    // No ownership transfer is needed.
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
  }

  // 6. Set other properties like transform, alpha blending, and presentation
  // mode.
  create_info.preTransform = swap_chain_support.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped =
      VK_TRUE; // We don't care about pixels that are obscured.

  // 7. If we're recreating the swap chain, link to the old one for faster
  // resource transition.
  create_info.oldSwapchain = m_old_swapchain == nullptr
                                 ? VK_NULL_HANDLE
                                 : m_old_swapchain->m_swap_chain;

  // 8. Create the swapchain object.
  if (vkCreateSwapchainKHR(m_device.getDeviceHandle(), &create_info, nullptr,
                           &m_swap_chain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  // 9. Retrieve the handles to the images created by the swap chain.
  vkGetSwapchainImagesKHR(m_device.getDeviceHandle(), m_swap_chain,
                          &image_count, nullptr);
  m_swap_chain_images.resize(image_count);
  vkGetSwapchainImagesKHR(m_device.getDeviceHandle(), m_swap_chain,
                          &image_count, m_swap_chain_images.data());

  // 10. Store the chosen format and extent for other parts of the renderer to
  // use.
  m_swap_chain_image_format = surface_format.format;
  m_swap_chain_extent = extent;
}

void VulkanSwapChain::createTextureWrappers() {
  // This commented-out block creates a VkImageView for each VkImage in the swap
  // chain. An image view is needed to tell Vulkan how to interpret the image
  // data (e.g., as a 2D color texture).
  auto FRAMES_IN_FLIGHT = getImageCount();
  m_swap_chain_texture_rids.resize(FRAMES_IN_FLIGHT); // Resize rids vector too
  m_swap_chain_depth_texture_rids.resize(FRAMES_IN_FLIGHT);
  for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
    auto texture = std::make_unique<VulkanTexture>(
        m_device, m_swap_chain_images[i], m_swap_chain_image_format);
    auto depth_texture = std::make_unique<VulkanTexture>(
        m_device, m_swap_chain_extent, findDepthFormat());
    m_swap_chain_texture_rids[i] = m_resource_manager.add(std::move(texture));
    m_swap_chain_depth_texture_rids[i] =
        m_resource_manager.add(std::move(depth_texture));
  }
}

RID VulkanSwapChain::getTextureRID(uint32_t index) const {
  return m_swap_chain_texture_rids[index];
}

RID VulkanSwapChain::getDepthTextureRID(uint32_t index) const {
  return m_swap_chain_depth_texture_rids[index];
}

void VulkanSwapChain::createRenderPass() {
  // A Render Pass tells Vulkan about the framebuffer attachments that will be
  // used during rendering. For now, we only have one color attachment.

  VkAttachmentDescription color_attachment = {};
  color_attachment.format =
      getSwapChainImageFormat(); // Use the swap chain's image format.
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp =
      VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the framebuffer before drawing a new
                                   // frame.
  color_attachment.storeOp =
      VK_ATTACHMENT_STORE_OP_STORE; // Store the rendered content so it can be
                                    // presented.
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout =
      VK_IMAGE_LAYOUT_UNDEFINED; // We don't care about the previous layout.
  color_attachment.finalLayout =
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Must be this layout for presentation.

  // A subpass is a sub-division of a render pass. For now, we only have one.
  VkAttachmentReference color_attachment_ref = {};
  color_attachment_ref.attachment =
      0; // Index of the attachment in the pAttachments array.
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;
  subpass.pDepthStencilAttachment = nullptr; // No depth buffer for now.

  // A subpass dependency controls the execution order and memory dependency
  // between subpasses. This one ensures that the render pass waits for the
  // image to be available before writing to it.
  VkSubpassDependency dependency = {};
  dependency.srcSubpass =
      VK_SUBPASS_EXTERNAL;   // Implicit subpass before the render pass
  dependency.dstSubpass = 0; // Our subpass (index 0)
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 1> attachments = {color_attachment};
  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
  render_pass_info.pAttachments = attachments.data();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;

  if (vkCreateRenderPass(m_device.getDeviceHandle(), &render_pass_info, nullptr,
                         &m_render_pass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void VulkanSwapChain::createFramebuffers() {
  // Create a framebuffer for each image view in the swap chain.
  // The framebuffer binds a specific set of attachments (like our color image
  // view) to a render pass.
  auto FRAMES_IN_FLIGHT = getImageCount();
  m_swap_chain_framebuffers.resize(FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
    auto texture =
        m_resource_manager.get_ptr<VulkanTexture>(m_swap_chain_texture_rids[i]);
    std::array<VkImageView, 1> attachments = {texture->getImageView()};

    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = m_render_pass;
    framebuffer_info.attachmentCount =
        static_cast<uint32_t>(attachments.size());
    framebuffer_info.pAttachments = attachments.data();
    framebuffer_info.width = m_swap_chain_extent.width;
    framebuffer_info.height = m_swap_chain_extent.height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(m_device.getDeviceHandle(), &framebuffer_info,
                            nullptr,
                            &m_swap_chain_framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void VulkanSwapChain::createSyncObjects() {
  // Create semaphores and fences to synchronize rendering and presentation.
  auto FRAMES_IN_FLIGHT = getImageCount();
  m_image_available_semaphores.resize(FRAMES_IN_FLIGHT);
  m_render_finished_semaphores.resize(FRAMES_IN_FLIGHT);
  m_in_flight_fences.resize(FRAMES_IN_FLIGHT);
  m_images_in_flight.resize(FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphore_info = {};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info = {};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags =
      VK_FENCE_CREATE_SIGNALED_BIT; // Create fences in a signaled state.

  for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(m_device.getDeviceHandle(), &semaphore_info, nullptr,
                          &m_image_available_semaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(m_device.getDeviceHandle(), &semaphore_info, nullptr,
                          &m_render_finished_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(m_device.getDeviceHandle(), &fence_info, nullptr,
                      &m_in_flight_fences[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "failed to create synchronization objects for a frame!");
    }
  }
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  // We look for a specific format (B8G8R8A8_SRGB) which is a common standard.
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }
  // If our preferred format is not available, we just return the first one
  // Vulkan offers.
  return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  // We prefer Mailbox mode (triple buffering) for low latency without tearing.
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      std::cout << "Present mode: Mailbox" << std::endl;
      return availablePresentMode;
    }
  }
  // If Mailbox is not available, we fall back to FIFO (standard V-Sync), which
  // is always available.
  std::cout << "Present mode: V-Sync" << std::endl;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR &capabilities) {
  // If the window manager gives us a specific extent, we must use it.
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    // Otherwise, we can choose our own, but it must be within the supported
    // min/max extents.
    VkExtent2D actualExtent = m_window_extent;
    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);
    return actualExtent;
  }
}

VkFormat VulkanSwapChain::findDepthFormat() {
  // This is a helper to find a supported depth format, not strictly needed for
  // Stage 2.
  return m_device.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // namespace Render::Vulkan
