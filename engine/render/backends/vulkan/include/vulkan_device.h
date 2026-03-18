#pragma once
#include "vulkan_types.h"
#include <vulkan/vulkan_raii.hpp>

// Forwarc declaration
namespace ssme {
class Platform;
}
namespace ssme::vulkan {


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
  /**
   * @brief Constructs the device, initializing handles to null.
   */
  VulkanDevice(ssme::Platform *platform);

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
  void initialize();

  // --- Getters for foundational Vulkan objects ---
  /** @brief Returns the logical device as vk::raii::Device reference. */
  vk::raii::Device const &getHandle() const { return m_device; }
  /** @brief Returns the physical device handle (the graphics card). */
  vk::PhysicalDevice getPhysicalDeviceHandle() const {
    return *m_physical_device;
  }
  /** @brief Returns the Vulkan instance handle, connecting the app to the
   * driver. */
  vk::Instance getInstanceHandle() const { return *m_instance; }
  /** @brief Returns the queue for graphics commands. */
  vk::raii::Queue getGraphicsQueue() const & { return m_graphics_queue; }
  /** @brief Returns the queue used for presentation (displaying images). */
  vk::raii::Queue getPresentQueue() const & { return m_present_queue; }
  /** @brief Returns the command pool used for allocating command buffers. */
  vk::CommandPool getCommandPool() const { return *m_command_pool; }
  /** @brief Returns the rendering surface (tied to the window). */
  vk::SurfaceKHR getSurface() const { return *m_surface; }

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
  QueueFamilyIndices getPhysicalQueueFamilies();
  /**
   * @brief Finds a supported format from a list of candidates. Used for
   * features like depth buffers.
   */
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates,
                                 vk::ImageTiling tiling,
                                 vk::FormatFeatureFlags features);
  /**
   * @brief Finds a suitable memory type index on the GPU (e.g., for
   * DEVICE_LOCAL memory).
   * @param type_filter A bitmask of suitable memory type indices.
   * @param properties The required memory properties (e.g., device local,
   * host visible).
   * @return The index of the found memory type.
   */
  uint32_t findMemoryType(uint32_t type_filter,
                          vk::MemoryPropertyFlags properties);
  /**
   * @brief A helper function to ...
   */
  vk::raii::CommandBuffer beginSingleTimeCommands();
  // VkCommandBuffer beginSingleTimeCommands();
  /**
   * @brief A helper function to ...
   */
  // void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void endSingleTimeCommands(vk::raii::CommandBuffer commandBuffer);
  /**
   * @brief A helper function to ...
   */
  void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width,
                         uint32_t height, uint32_t layerCount);
  /**
   * @brief A helper function to ...
   */
  void transitionImageLayout(vk::Image image, vk::Format format,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout);

  void pipelineBarrier(const vk::raii::CommandBuffer &cmd_buffer,
                       vk::Image image, vk::Format format,
                       vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                       vk::AccessFlags2 src_access_mask,
                       vk::AccessFlags2 dst_access_mask,
                       vk::PipelineStageFlags2 src_stage_mask,
                       vk::PipelineStageFlags2 dst_stage_mask);
  /**
   * @brief A helper function to create a VkImage.
   */
  ImageResource createImage(const vk::ImageCreateInfo &imageInfo,
                            vk::MemoryPropertyFlags properties);
  /**
   * @brief A helper function to create a VkImageView.
   * An image view describes how to access an image and which part of the image
   * to access.
   */
  vk::raii::ImageView createImageView(vk::Image image, vk::Format format,
                                      vk::ImageAspectFlagBits flags);
  /**
   * @brief A helper function to create a VkDeviceMemory.
   */
  BufferResource createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                              vk::MemoryPropertyFlags properties);
  /**
   * @brief A helper function to ...
   */
  vk::raii::Sampler createTextureSampler();

  vk::raii::CommandBuffer createCommandBuffer() const;

private:
  /**
   * @brief Creates the Vulkan instance, which connects the application to the
   * Vulkan driver.
   * @param required_extensions A list of required instance-level extensions
   * (e.g., for surfaces and debug utils).
   */
  void createInstance(const std::vector<const char *> &required_extensions,
                      const std::vector<const char *> &required_layers);
  /**
   * @brief Sets up the debug messenger callback for receiving validation layer
   * messages. Only active in debug builds.
   */
  void setupDebugMessenger();

  void createSurface();

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
  bool isDeviceSuitable(const vk::raii::PhysicalDevice &device);
  /**
   * @brief A helper for isDeviceSuitable that checks if a device supports
   * required extensions (e.g., swapchain).
   */
  bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice &device);
  /**
   * @brief A helper for isDeviceSuitable that finds supported queue families on
   * a device.
   */
  QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice &device,
                                       vk::SurfaceKHR surface);
  /**
   * @brief A helper that queries the detailed swap chain support for a given
   * device and surface.
   */
  SwapChainSupportDetails
  querySwapChainSupport(const vk::raii::PhysicalDevice &device);

  ssme::Platform *m_platform;
  // --- Vulkan RAII Objects ---
  vk::raii::Context m_context;
  vk::raii::Instance m_instance = nullptr;
  vk::raii::DebugUtilsMessengerEXT m_debug_messenger = nullptr;
  vk::raii::SurfaceKHR m_surface = nullptr;
  vk::raii::PhysicalDevice m_physical_device = nullptr;
  vk::raii::Device m_device = nullptr;
  vk::raii::Queue m_graphics_queue = nullptr;
  vk::raii::Queue m_present_queue = nullptr;
  vk::raii::CommandPool m_command_pool = nullptr;
  // --- Configuration ---
  const std::vector<const char *> m_validation_layers = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> m_device_extensions = {
      vk::KHRSwapchainExtensionName, vk::KHRDynamicRenderingExtensionName};

  QueueFamilyIndices m_queue_family_indices;
#ifndef NDEBUG
  bool g_enable_validation_layers = true;
#else
  bool g_enable_validation_layers = false;
#endif
};

} // namespace ssme::vulkan
