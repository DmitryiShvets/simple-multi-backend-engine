#pragma once
#include "i_surface_creator.h"
#include "vulkan_types.h"

namespace Render::Vulkan {

/**
 * @class VulkanDevice
 * @brief A low-level class that encapsulates the physical hardware and
 * foundational Vulkan objects.
 *
 * This class is responsible for the entire initial setup of the Vulkan API:
 * 1. Creating the `VkInstance` (the connection to the Vulkan driver).
 * 2. Selecting a physical device (`VkPhysicalDevice`, i.e., the GPU).
 * 3. Creating a logical device (`VkDevice`) that the rest of the renderer
 * interacts with.
 * 4. Finding and creating command queues (`VkQueue`) for submitting work to the
 * GPU.
 *
 * It represents direct control over the Vulkan state and is not part of the
 * public RHI. Other Vulkan-specific classes (e.g., ResourceManager, SwapChain)
 * use it to access the `VkDevice`.
 */
class VulkanDevice {
public:
  PFN_vkCmdBeginRenderingKHR pfn_vkCmdBeginRenderingKHR = nullptr;
  PFN_vkCmdEndRenderingKHR pfn_vkCmdEndRenderingKHR = nullptr;

  /**
   * @brief Constructs the device, initializing handles to null.
   */
  VulkanDevice();

  /**
   * @brief Destroys all created Vulkan objects in the correct order.
   */
  ~VulkanDevice();

  /**
   * @brief Main initialization method that triggers the entire Vulkan setup
   * chain.
   * @param creator A strategy object for creating the platform-specific
   * VkSurfaceKHR.
   */
  void initialize(IVulkanSurfaceCreator &creator);

  // --- Getters for foundational Vulkan objects ---

  /** @brief Returns the logical device handle, the main object for GPU
   * interaction. */
  VkDevice getDeviceHandle() const { return m_device; }
  /** @brief Returns the physical device handle (the graphics card). */
  VkPhysicalDevice getPhysicalDeviceHandle() const { return m_physical_device; }
  /** @brief Returns the Vulkan instance handle, connecting the app to the
   * driver. */
  VkInstance getInstanceHandle() const { return m_instance; }
  /** @brief Returns the queue for graphics commands. */
  VkQueue getGraphicsQueue() const { return m_graphics_queue; }
  /** @brief Returns the queue used for presentation (displaying images). */
  VkQueue getPresentQueue() const { return m_present_queue; }
  /** @brief Returns the command pool used for allocating command buffers. */
  VkCommandPool getCommandPool() const { return m_command_pool; }
  /** @brief Returns the rendering surface (tied to the window). */
  VkSurfaceKHR getSurface() const { return m_surface; }

  // --- Public Utility Methods ---

  /**
   * @brief Queries the physical device for its swap chain creation
   * capabilities.
   * @return A struct containing details on supported formats, present modes,
   * etc.
   */
  SwapChainSupportDetails getSwapChainSupport();
  /**
   * @brief Finds the indices of the required queue families (graphics and
   * presentation).
   * @return A struct containing the queue family indices.
   */
  QueueFamilyIndices findPhysicalQueueFamilies();
  /**
   * @brief Finds a supported format from a list of candidates. Used for
   * features like depth buffers.
   */
  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);
  /**
   * @brief Finds a suitable memory type index on the GPU (e.g., for
   * DEVICE_LOCAL memory).
   * @param type_filter A bitmask of suitable memory type indices.
   * @param properties The required memory properties (e.g., device local, host
   * visible).
   * @return The index of the found memory type.
   */
  uint32_t findMemoryType(uint32_t type_filter,
                          VkMemoryPropertyFlags properties);
  /**
   * @brief A helper function to ...
   */
  VkCommandBuffer beginSingleTimeCommands();
  /**
   * @brief A helper function to ...
   */
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  /**
   * @brief A helper function to ...
   */
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height, uint32_t layerCount);
  /**
   * @brief A helper function to ...
   */
  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout);
  /**
   * @brief A helper function to create a VkImage.
   */
  void createImage(const VkImageCreateInfo &imageInfo,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory);
  /**
   * @brief A helper function to create a VkImageView.
   * An image view describes how to access an image and which part of the image
   * to access.
   */
  VkImageView createImageView(VkImage image, VkFormat format);
  /**
   * @brief A helper function to create a VkDeviceMemory.
   */
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory);
  /**
   * @brief A helper function to ...
   */
  void createTextureSampler(VkSampler &textureSampler);

private:
  /**
   * @brief Creates the Vulkan instance, which connects the application to the
   * Vulkan driver.
   * @param required_extensions A list of required instance-level extensions
   * (e.g., for surfaces and debug utils).
   */
  void createInstance(const std::vector<const char *> &required_extensions);
  /**
   * @brief Sets up the debug messenger callback for receiving validation layer
   * messages. Only active in debug builds.
   */
  void setupDebugMessenger();
  /**
   * @brief Enumerates available physical devices (GPUs) and selects the most
   * suitable one.
   */
  void pickPhysicalDevice();
  /**
   * @brief Creates the logical device (`VkDevice`), which is the primary
   * interface for interacting with the selected GPU.
   */
  void createLogicalDevice();
  /**
   * @brief Creates a command pool, from which command buffers are allocated.
   */
  void createCommandPool();

  /**
   * @brief Checks if a given physical device is suitable for the application's
   * needs.
   * @param device The device to check.
   * @return True if the device is suitable, false otherwise.
   */
  bool isDeviceSuitable(VkPhysicalDevice device);
  /**
   * @brief A helper for isDeviceSuitable that checks if a device supports
   * required extensions (e.g., swapchain).
   */
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  /**
   * @brief A helper for isDeviceSuitable that finds supported queue families on
   * a device.
   */
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
                                       VkSurfaceKHR surface);
  /**
   * @brief A helper that queries the detailed swap chain support for a given
   * device and surface.
   */
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

  // --- Vulkan Object Handles ---
  VkInstance m_instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
  VkPhysicalDevice m_physical_device = VK_NULL_HANDLE; // The GPU
  VkDevice m_device = VK_NULL_HANDLE;                  // The logical device
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;
  VkCommandPool m_command_pool = VK_NULL_HANDLE;

  VkQueue m_graphics_queue = VK_NULL_HANDLE;
  VkQueue m_present_queue = VK_NULL_HANDLE;

  // --- Configuration ---
  const std::vector<const char *> m_validation_layers = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> m_device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};
};

} // namespace Render::Vulkan
