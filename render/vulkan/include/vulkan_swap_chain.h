#pragma once

#include "vulkan_resource_manager.h"

#include "vulkan_device.h"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace Render::Vulkan {

/**
 * @class VulkanSwapChain
 * @brief Manages the Vulkan swap chain and its associated resources.
 *
 * The swap chain is a queue of images that are waiting to be presented to the
 * screen. This class encapsulates the `VkSwapchainKHR` object and handles:
 * - Creation of the swap chain itself.
 * - Management of the images (`VkImage`) owned by the swap chain.
 * - Creation of `VkImageView`s to view those images.
 * - Creation of a compatible `VkRenderPass`.
 * - Creation of `VkFramebuffer`s for rendering into the images.
 * - Management of synchronization objects (semaphores, fences) to control the
 * rendering and presentation loop.
 */
class VulkanSwapChain {
public:
  /**
   * @brief Constructs a new swap chain.
   * @param deviceRef A reference to the VulkanDevice.
   * @param windowExtent The width and height of the window.
   */
  VulkanSwapChain(VulkanDevice &deviceRef, VkExtent2D windowExtent,
                  VulkanResourceManager &resourceManager);
  /**
   * @brief Re-creates a swap chain, linking it to a previous one (e.g., after a
   * window resize). This allows for faster recreation and resource sharing.
   * @param deviceRef A reference to the VulkanDevice.
   * @param windowExtent The new width and height of the window.
   * @param previous A shared pointer to the old swap chain that is being
   * replaced.
   */
  VulkanSwapChain(VulkanDevice &deviceRef, VkExtent2D windowExtent,
                  std::shared_ptr<VulkanSwapChain> previous,
                  VulkanResourceManager &resourceManager);
  ~VulkanSwapChain();

  // Disable copy and assignment to prevent accidental duplication of this
  // heavyweight object.
  VulkanSwapChain(const VulkanSwapChain &) = delete;
  VulkanSwapChain &operator=(const VulkanSwapChain &) = delete;

  // --- Getters ---
  size_t getCurrentFrameIndex() const { return m_current_frame; }
  VkFramebuffer getFrameBuffer(int index) {
    return m_swap_chain_framebuffers[index];
  }
  VkRenderPass getRenderPass() { return m_render_pass; }
  size_t getImageCount() { return m_swap_chain_images.size(); }
  VkFormat getSwapChainImageFormat() { return m_swap_chain_image_format; }
  VkExtent2D getSwapChainExtent() { return m_swap_chain_extent; }
  uint32_t width() { return m_swap_chain_extent.width; }
  uint32_t height() { return m_swap_chain_extent.height; }
  float extentAspectRatio() {
    return static_cast<float>(m_swap_chain_extent.width) /
           static_cast<float>(m_swap_chain_extent.height);
  }

  RID getTextureRID(uint32_t index) const;
  RID getDepthTextureRID(uint32_t index) const;
  VkImage getImage(uint32_t index) const;
  VkImageView getImageView(uint32_t index) const;

  /**
   * @brief Finds a suitable depth format supported by the physical device.
   */
  VkFormat findDepthFormat();

  /**
   * @brief Acquires the index of the next available image from the swap chain
   * to be rendered into.
   * @param imageIndex A pointer to a uint32_t that will be filled with the
   * acquired image index.
   * @return A VkResult indicating success, or if the swap chain is out of date
   * and needs recreation.
   */
  VkResult acquireNextImage(uint32_t *imageIndex);

  /**
   * @brief Submits the provided command buffers for execution and presents the
   * rendered image to the screen.
   * @param buffers A pointer to an array of command buffers to be submitted.
   * @param imageIndex A pointer to the index of the image that was just
   * rendered into.
   * @return A VkResult indicating success or if the swap chain is out of date.
   */
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers,
                                uint32_t *imageIndex);

private:
  /** @brief Main initialization function that calls all the creation helpers.
   */
  void init();
  /** @brief Creates the core `VkSwapchainKHR` object. */
  void createSwapChain();
  /** @brief Creates a VulkanTexture wrapper for each VkImage in the swap chain. */
  void createTextureWrappers();
  // void createDepthResources();
  /** @brief Creates the render pass, defining the attachments, subpasses, and
   * dependencies. */
  void createRenderPass();
  /** @brief Creates a framebuffer for each image view in the swap chain. */
  void createFramebuffers();
  /** @brief Creates synchronization objects (semaphores and fences) needed for
   * the frame loop. */
  void createSyncObjects();

  // Helper functions for selecting optimal swap chain settings.
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  // --- References and State ---
  VulkanDevice &m_device;
  VkExtent2D m_window_extent;
  std::shared_ptr<VulkanSwapChain> m_old_swapchain;
  size_t m_current_frame = 0;

  // --- Swap Chain Properties ---
  VkFormat m_swap_chain_image_format;
  VkExtent2D m_swap_chain_extent;

  // --- Vulkan Objects ---
  VkSwapchainKHR m_swap_chain;
  VkRenderPass m_render_pass;
  std::vector<VkImage> m_swap_chain_images;
  std::vector<RID> m_swap_chain_texture_rids;
  std::vector<RID> m_swap_chain_depth_texture_rids;
  std::vector<VkFramebuffer> m_swap_chain_framebuffers;

  // --- Synchronization Objects ---
  std::vector<VkSemaphore> m_image_available_semaphores;
  std::vector<VkSemaphore> m_render_finished_semaphores;
  std::vector<VkFence> m_in_flight_fences;
  std::vector<VkFence> m_images_in_flight;

  //  --- Resource Objects ---
  VulkanResourceManager &m_resource_manager;
};

} // namespace Render::Vulkan
