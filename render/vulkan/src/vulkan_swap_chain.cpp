#include "vulkan_swap_chain.h"
#include "vulkan_texture.h"
#include "vulkan_types.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace Render::Vulkan {

VulkanSwapChain::VulkanSwapChain(VulkanDevice &deviceRef, vk::Extent2D extent,
                                 VulkanResourceManager &resourceManager)
    : m_device{deviceRef}, m_window_extent{extent},
      m_resource_manager(resourceManager) {
  init();
}

// TDOD: ADD RECREATION SWAPCHAIN
VulkanSwapChain::VulkanSwapChain(VulkanDevice &deviceRef,
                                 vk::Extent2D windowExtent,
                                 std::shared_ptr<VulkanSwapChain> previous,
                                 VulkanResourceManager &resourceManager)
    : m_device{deviceRef}, m_window_extent{windowExtent},
      m_old_swapchain{previous}, m_resource_manager(resourceManager) {
  init();
  // The old swapchain is no longer needed after its resources are reused.
  m_old_swapchain = nullptr;
}

VulkanSwapChain::~VulkanSwapChain() {}

void VulkanSwapChain::init() {
  createSwapChain();
  createTextureWrappers();
  createSyncObjects();
}

vk::Result VulkanSwapChain::acquireNextImage(uint32_t *imageIndex) {
  // Wait until the fence associated with the current frame is signaled.
  // This means the GPU has finished rendering the frame from a previous loop.
  // This prevents the CPU from getting more than FRAMES_IN_FLIGHT frames
  // ahead of the GPU.
  auto fence_result = m_device.getHandle().waitForFences(
      *m_in_flight_fences[m_current_frame], vk::True, UINT64_MAX);
  // Reset the fence to an unsignaled state before using it.
  m_device.getHandle().resetFences(*m_in_flight_fences[m_current_frame]);
  // Acquire the next available image from the swapchain.
  auto [result, image_index] = m_swap_chain.acquireNextImage(
      UINT64_MAX, m_image_available_semaphores[m_current_frame], nullptr);
  *imageIndex = image_index;
  return result;
}

vk::Result
VulkanSwapChain::submitCommandBuffers(const vk::CommandBuffer *buffers,
                                      uint32_t *imageIndex) {
  // --- Configure the submission ---
  // Specify which semaphores to signal once the command buffer has finished
  // execution.
  vk::Semaphore wait_semaphores[] = {
      *m_image_available_semaphores[m_current_frame]};
  vk::PipelineStageFlags wait_stages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::Semaphore signal_semaphores[] = {
      *m_render_finished_semaphores[*imageIndex]};

  vk::SubmitInfo submit_info{.waitSemaphoreCount = 1,
                             .pWaitSemaphores = wait_semaphores,
                             .pWaitDstStageMask = wait_stages,
                             .commandBufferCount = 1,
                             .pCommandBuffers = buffers,
                             .signalSemaphoreCount = 1,
                             .pSignalSemaphores = signal_semaphores};

  // Submit the command buffer to the graphics queue.
  // The `inFlightFences` will be signaled when the command buffer has finished
  // execution.
  m_device.getGraphicsQueue().submit(submit_info,
                                     m_in_flight_fences[m_current_frame]);

  // --- Configure the presentation ---
  // Specify the swap chain to present to.
  vk::SwapchainKHR swap_chains[] = {*m_swap_chain};

  vk::PresentInfoKHR present_info = {.waitSemaphoreCount = 1,
                                     .pWaitSemaphores = signal_semaphores,
                                     .swapchainCount = 1,
                                     .pSwapchains = swap_chains,
                                     .pImageIndices = imageIndex};
  // Wait on the `renderFinishedSemaphores` before presentation can happen.
  // Submit the request to present the image to the screen.
  auto result = m_device.getPresentQueue().presentKHR(present_info);
  // Advance to the next frame index.
  m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void VulkanSwapChain::createSwapChain() {
  // This commented-out block contains the logic for creating the vk::SwapchainKHR
  // object.

  // 1. Get all the details about what the physical device and surface support.
  SwapChainSupportDetails swap_chain_support = m_device.getSwapChainSupport();

  // 2. Choose the best settings from the available options.
  vk::SurfaceFormatKHR surface_format =
      chooseSwapSurfaceFormat(swap_chain_support.formats);
  vk::PresentModeKHR present_mode =
      chooseSwapPresentMode(swap_chain_support.present_modes);
  vk::Extent2D extent = chooseSwapExtent(swap_chain_support.capabilities);

  // 3. Decide on the number of images in the swap chain. It's good practice to
  // request at least one more than the minimum to avoid waiting on the driver.
  uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
  if (swap_chain_support.capabilities.maxImageCount > 0 &&
      image_count > swap_chain_support.capabilities.maxImageCount) {
    image_count = swap_chain_support.capabilities.maxImageCount;
  }

  // 4. Fill out the main creation structure.
  vk::SwapchainCreateInfoKHR create_info{
      .flags = vk::SwapchainCreateFlagsKHR(),
      .surface = m_device.getSurface(),
      .minImageCount = image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eTransferDst,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = swap_chain_support.capabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = present_mode,
      .clipped = vk::True,
      .oldSwapchain = nullptr};
  // 5. Handle how images are used across different queue families.
  QueueFamilyIndices indices = m_device.getPhysicalQueueFamilies();
  uint32_t queue_family_indices[] = {indices.graphics_family,
                                     indices.present_family};

  if (indices.graphics_family != indices.present_family) {
    // If graphics and present queues are different, we use CONCURRENT mode.
    // This is less performant but easier to manage.
    create_info.imageSharingMode = vk::SharingMode::eConcurrent;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    // If they are the same, we use EXCLUSIVE mode, which is more performant.
    // No ownership transfer is needed.
    create_info.imageSharingMode = vk::SharingMode::eExclusive;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
  }

  // 6. Set other properties like transform, alpha blending, and presentation

  // 7. If we're recreating the swap chain, link to the old one for faster
  // resource transition.
  create_info.oldSwapchain =
      m_old_swapchain == nullptr ? nullptr : *m_old_swapchain->m_swap_chain;

  // 8. Create the swapchain object.
  m_swap_chain = vk::raii::SwapchainKHR(m_device.getHandle(), create_info);
  m_swap_chain_images = m_swap_chain.getImages();
  // 9. Store the chosen format and extent for other parts of the renderer to
  // use.
  m_swap_chain_image_format = surface_format.format;
  m_swap_chain_extent = extent;
}

void VulkanSwapChain::createTextureWrappers() {
  // This commented-out block creates a vk::ImageView for each vk::Image in the swap
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

void VulkanSwapChain::createSyncObjects() {
  // Create semaphores and fences to synchronize rendering and presentation.
  constexpr auto FRAMES_IN_FLIGHT = MAX_FRAMES_IN_FLIGHT;
  auto IMAGE_COUNT = getImageCount();

  assert(m_image_available_semaphores.empty() &&
         m_render_finished_semaphores.empty() && m_in_flight_fences.empty());

  vk::FenceCreateInfo fence_info{.flags = vk::FenceCreateFlagBits::eSignaled};

  // Create semaphores for each IMAGE (signal after rendering)
  for (size_t i = 0; i < IMAGE_COUNT; i++) {
    m_render_finished_semaphores.emplace_back(m_device.getHandle(),
                                              vk::SemaphoreCreateInfo());
  }
  // Create semaphores and fences for each FRAME (wait before rendering)
  for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
    m_image_available_semaphores.emplace_back(m_device.getHandle(),
                                              vk::SemaphoreCreateInfo());
    m_in_flight_fences.emplace_back(m_device.getHandle(), fence_info);
  }
}

vk::SurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &available_formats) {
  // We look for a specific format (B8G8R8A8_SRGB) which is a common standard.
  for (const auto &format : available_formats) {
    if (format.format == vk::Format::eB8G8R8A8Srgb &&
        format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return format;
    }
  }
  // If our preferred format is not available, we just return the first one
  // Vulkan offers.
  return available_formats[0];
}

vk::PresentModeKHR VulkanSwapChain::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &available_present_modes) {
  // We prefer Mailbox mode (triple buffering) for low latency without tearing.
  for (const auto &present_mode : available_present_modes) {
    if (present_mode == vk::PresentModeKHR::eMailbox) {
      return present_mode;
    }
  }
  // If Mailbox is not available, we fall back to FIFO (standard V-Sync), which
  // is always available.
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanSwapChain::chooseSwapExtent(
    const vk::SurfaceCapabilitiesKHR &capabilities) {
  // If the window manager gives us a specific extent, we must use it.
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    // Otherwise, we can choose our own, but it must be within the supported
    // min/max extents.
    vk::Extent2D extent = m_window_extent;
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height =
        std::clamp(extent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);
    return extent;
  }
}

vk::Format VulkanSwapChain::findDepthFormat() {
  // This is a helper to find a supported depth format, not strictly needed for
  // Stage 2.
  return m_device.findSupportedFormat(
      {
          vk::Format::eD32Sfloat,
          vk::Format::eD32Sfloat,
          vk::Format::eD32SfloatS8Uint,

      },
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

} // namespace Render::Vulkan
