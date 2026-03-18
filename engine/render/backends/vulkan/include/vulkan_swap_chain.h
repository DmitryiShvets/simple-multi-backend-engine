#pragma once

#include "vulkan_resource_manager.h"
#include "vulkan_device.h"

#include <memory>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace ssme::vulkan {

// Maximum number of frames that can be processed concurrently
// This is independent of the number of swap chain images
constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;

/**
 * @class VulkanSwapChain
 * @brief Manages the Vulkan swap chain and its associated resources.
 *
 * The swap chain is a queue of images that are waiting to be presented to the
 * screen. This class encapsulates the `VkSwapchainKHR` object and handles:
 * - Creation of the swap chain itself.
 * - Management of the images (`vk::Image`) owned by the swap chain.
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
  VulkanSwapChain(VulkanDevice &deviceRef, vk::Extent2D windowExtent,
                  VulkanResourceManager &resourceManager);
  /**
   * @brief Re-creates a swap chain, linking it to a previous one (e.g., after a
   * window resize). This allows for faster recreation and resource sharing.
   * @param deviceRef A reference to the VulkanDevice.
   * @param windowExtent The new width and height of the window.
   * @param previous A shared pointer to the old swap chain that is being
   * replaced.
   */
  VulkanSwapChain(VulkanDevice &deviceRef, vk::Extent2D windowExtent,
                  std::shared_ptr<VulkanSwapChain> previous,
                  VulkanResourceManager &resourceManager);
  ~VulkanSwapChain();

  // Disable copy and assignment to prevent accidental duplication of this
  // heavyweight object.
  VulkanSwapChain(const VulkanSwapChain &) = delete;
  VulkanSwapChain &operator=(const VulkanSwapChain &) = delete;

  // --- Getters ---
  size_t getCurrentFrameIndex() const { return m_current_frame; }
  size_t getImageCount() { return m_swap_chain_images.size(); }
  vk::Format getSwapChainImageFormat() { return m_swap_chain_image_format; }
  vk::Extent2D getSwapChainExtent() { return m_swap_chain_extent; }
  uint32_t width() { return m_swap_chain_extent.width; }
  uint32_t height() { return m_swap_chain_extent.height; }
  float extentAspectRatio() {
    return static_cast<float>(m_swap_chain_extent.width) /
           static_cast<float>(m_swap_chain_extent.height);
  }

  RID getTextureRID(uint32_t index) const;
  RID getDepthTextureRID(uint32_t index) const;
  vk::Image getImage(uint32_t index) const;
  vk::ImageView getImageView(uint32_t index) const;

  /**
   * @brief Finds a suitable depth format supported by the physical device.
   */
  vk::Format findDepthFormat();

  /**
   * @brief Acquires the index of the next available image from the swap chain
   * to be rendered into.
   * @param imageIndex A pointer to a uint32_t that will be filled with the
   * acquired image index.
   * @return A vk::Result indicating success, or if the swap chain is out of date
   * and needs recreation.
   */
  vk::Result acquireNextImage(
      uint32_t *imageIndex); // wait for fences and retrives new image

  /**
   * @brief Submits the provided command buffers for execution and presents the
   * rendered image to the screen.
   * @param buffers A pointer to an array of command buffers to be submitted.
   * @param imageIndex A pointer to the index of the image that was just
   * rendered into.
   * @return A vk::Result indicating success or if the swap chain is out of date.
   */
  vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers,
                                uint32_t *imageIndex);

private:
  /** @brief Main initialization function that calls all the creation helpers.
   */
  void init();
  /** @brief Creates the core `VkSwapchainKHR` object. */
  void createSwapChain();
  /** @brief Creates a VulkanTexture wrapper for each VkImage in the swap chain.
   */
  void createTextureWrappers();
  // void createDepthResources();
  /** @brief Creates the render pass, defining the attachments, subpasses, and
   * dependencies. */
  // void createRenderPass();
  /** @brief Creates a framebuffer for each image view in the swap chain. */
  // void createFramebuffers();
  /** @brief Creates synchronization objects (semaphores and fences) needed for
   * the frame loop. */
  void createSyncObjects();

  // Helper functions for selecting optimal swap chain settings.
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> &available_formats);
  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &available_present_modes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

  // --- References and State ---
  VulkanDevice &m_device;
  vk::Extent2D m_window_extent;
  std::shared_ptr<VulkanSwapChain> m_old_swapchain;
  size_t m_current_frame = 0;

  // --- Swap Chain Properties ---
  vk::Format m_swap_chain_image_format;
  vk::Extent2D m_swap_chain_extent;
  // --- Vulkan RAII Objects ---
  vk::raii::SwapchainKHR m_swap_chain = nullptr;

  // --- Vulkan Object Handles ---
  std::vector<vk::Image> m_swap_chain_images;
  std::vector<RID> m_swap_chain_texture_rids;
  std::vector<RID> m_swap_chain_depth_texture_rids;

  // --- Synchronization Objects ---
  // Семантика индексации:
  // - m_image_available_semaphores[frameIndex]: ждём ПЕРЕД рендерингом кадра
  // - m_render_finished_semaphores[imageIndex]: сигналим ПОСЛЕ рендеринга
  // изображения
  // - m_in_flight_fences[frameIndex]: fence для кадра (CPU vs GPU)
  // - m_images_in_flight[imageIndex]: fence для изображения (какое изображение
  // занято)
  std::vector<vk::raii::Semaphore>
      m_image_available_semaphores; // MAX_FRAMES_IN_FLIGHT
  std::vector<vk::raii::Semaphore>
      m_render_finished_semaphores;                // swapChainImages.size()
  std::vector<vk::raii::Fence> m_in_flight_fences; // MAX_FRAMES_IN_FLIGHT
  //  --- Resource Objects ---
  VulkanResourceManager &m_resource_manager;
};

} // namespace ssme::vulkan
