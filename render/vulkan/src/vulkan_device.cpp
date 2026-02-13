#include "vulkan_device.h"

#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>

namespace Render::Vulkan {

// These are local helper functions and are not part of the VulkanDevice class.
// They are defined as static to limit their scope to this translation unit
// (.cpp file).

/**
 * @brief This is the callback function that Vulkan's validation layers will
 * call. Whenever a validation rule is broken, this function is invoked with
 * details about the error.
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
              VkDebugUtilsMessageTypeFlagsEXT message_type,
              const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
              void *p_user_data) {
  std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;
  return VK_FALSE; // Should always return VK_FALSE
}

/**
 * @brief Helper function to load the address of
 * `vkCreateDebugUtilsMessengerEXT`. This function is an extension and is not
 * loaded by default, so we must query its address manually.
 */
VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *p_create_info,
    const VkAllocationCallbacks *p_allocator,
    VkDebugUtilsMessengerEXT *p_debug_messenger) {
  // Get the function pointer from the Vulkan driver
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, p_create_info, p_allocator, p_debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

/**
 * @brief Helper function to load the address of
 * `vkDestroyDebugUtilsMessengerEXT`.
 */
void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debug_messenger,
                                   const VkAllocationCallbacks *p_allocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debug_messenger, p_allocator);
  }
}

// --- VulkanDevice Method Implementations ---

VulkanDevice::VulkanDevice() {
  // The constructor is kept lightweight.
  // The previous call to setupDebugMessenger() here was incorrect,
  // as it requires a valid VkInstance, which is not created until initialize()
  // is called.
}

VulkanDevice::~VulkanDevice() {
  // Destruction must happen in the reverse order of creation.
  vkDestroyCommandPool(m_device, m_command_pool, nullptr);
  vkDestroyDevice(m_device, nullptr);

#ifndef NDEBUG
  // The debug messenger is an instance-level object and should be destroyed
  // before the instance.
  destroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
#endif

  // The surface is also tied to the instance.
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  vkDestroyInstance(m_instance, nullptr);
}

void VulkanDevice::initialize(IVulkanSurfaceCreator &creator) {
  // This is the main entry point that orchestrates the entire setup process.
  // 1. Get required extensions from the surface creator (e.g., for GLFW).
  auto extensions = creator.getRequiredInstanceExtensions();

  // 2. In debug builds, add the extension needed for the debug messenger.
#ifndef NDEBUG
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

  // 3. Create the Vulkan Instance.
  createInstance(extensions);

  // 4. Create the window surface using the strategy object.
  if (creator.createWindowSurface(m_instance, &m_surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }

  // 5. Select a physical GPU.
  pickPhysicalDevice();
  // 6. Create a logical device to interface with the GPU.
  createLogicalDevice();
  // 7. Create a pool for command buffers.
  createCommandPool();
}

void VulkanDevice::createInstance(
    const std::vector<const char *> &required_extensions) {
  // --- Check for Validation Layer support in debug builds ---
#ifndef NDEBUG
  const bool enable_validation_layers = true;
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char *layer_name : m_validation_layers) {
    bool layer_found = false;
    for (const auto &layer_properties : available_layers) {
      if (strcmp(layer_name, layer_properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }
    if (!layer_found) {
      throw std::runtime_error(
          "validation layers requested, but not available!");
    }
  }
#else
  const bool enable_validation_layers = false;
#endif

  // --- Fill out Application Info ---
  // This struct is optional but provides good information to the driver.
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Primitives App";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Primitives";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_2;

  // --- Fill out Instance Create Info ---
  // This is the main struct for creating the instance.
  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions.size());
  create_info.ppEnabledExtensionNames = required_extensions.data();

  // --- Setup Debug Messenger (for createInstance call) ---
  // The debug messenger needs to be active even during instance
  // creation/destruction. So we pass its create info in the pNext chain of
  // VkInstanceCreateInfo.
  VkDebugUtilsMessengerCreateInfoEXT
      debug_create_info{}; // Must be outside the if scope
  if (enable_validation_layers) {
    create_info.enabledLayerCount =
        static_cast<uint32_t>(m_validation_layers.size());
    create_info.ppEnabledLayerNames = m_validation_layers.data();

    // Populate the debug messenger struct
    debug_create_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = debugCallback;
    debug_create_info.pUserData = nullptr;
    create_info.pNext = &debug_create_info;
  } else {
    create_info.enabledLayerCount = 0;
    create_info.pNext = nullptr;
  }

  // Finally, create the instance!
  if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }

  // After creating the instance, we also need to create the messenger object
  // itself.
  setupDebugMessenger();
}

void VulkanDevice::setupDebugMessenger() {
#ifndef NDEBUG
  // This method creates the debug messenger object.
  // Its creation info was passed during instance creation via the pNext chain,
  // but the object handle itself must be created after the instance exists.
  VkDebugUtilsMessengerCreateInfoEXT create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = debugCallback;
  create_info.pUserData = nullptr;

  if (createDebugUtilsMessengerEXT(m_instance, &create_info, nullptr,
                                   &m_debug_messenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
#endif
}

void VulkanDevice::pickPhysicalDevice() {
  // Get the list of all available GPUs with Vulkan support.
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
  if (device_count == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

  // Iterate through the GPUs and pick the first one that is suitable.
  // A more advanced implementation could score devices and pick the best one
  // (e.g., prefer discrete GPUs).
  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
      m_physical_device = device;
      break;
    }
  }

  if (m_physical_device == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) {
  // A device is suitable if it supports the queue families and extensions we
  // need.
  QueueFamilyIndices indices = findQueueFamilies(device, m_surface);
  bool extensions_supported = checkDeviceExtensionSupport(device);

  // It must support both graphics/present queues and the swapchain extension.
  // A full implementation would also check for swapchain adequacy (e.g., at
  // least one format and present mode available).
  return indices.isComplete() && extensions_supported;
}

bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  // Check if the device supports all the extensions we require (e.g.,
  // VK_KHR_SWAPCHAIN_EXTENSION_NAME).
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       nullptr);
  std::vector<VkExtensionProperties> available_extensions(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       available_extensions.data());

  std::set<std::string> required_extensions(m_device_extensions.begin(),
                                            m_device_extensions.end());
  for (const auto &extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device,
                                                   VkSurfaceKHR surface) {
  // Find queue families that support graphics operations and presentation to
  // our surface.
  QueueFamilyIndices indices;

  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           queue_families.data());

  int i = 0;
  for (const auto &queue_family : queue_families) {
    // Check for graphics support
    if (queue_family.queueCount > 0 &&
        queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
      indices.graphics_family_has_value = true;
    }

    // Check for presentation support to the window surface
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (queue_family.queueCount > 0 && present_support) {
      indices.present_family = i;
      indices.present_family_has_value = true;
    }

    if (indices.isComplete()) {
      break;
    }
    i++;
  }
  return indices;
}

void VulkanDevice::createLogicalDevice() {
  // Create the VkDevice, which is our main interface to the physical device.
  QueueFamilyIndices indices = findQueueFamilies(m_physical_device, m_surface);

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> unique_queue_families = {indices.graphics_family,
                                              indices.present_family};

  float queue_priority = 1.0f;
  for (uint32_t queue_family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  VkPhysicalDeviceFeatures device_features{};
  device_features.samplerAnisotropy = VK_TRUE;

  // --- Enable Dynamic Rendering Feature ---
  VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature{};
  dynamic_rendering_feature.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
  dynamic_rendering_feature.dynamicRendering = VK_TRUE;

  VkDeviceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount =
      static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.pEnabledFeatures = &device_features;
  create_info.enabledExtensionCount =
      static_cast<uint32_t>(m_device_extensions.size());
  create_info.ppEnabledExtensionNames = m_device_extensions.data();
  create_info.pNext = &dynamic_rendering_feature; // Chain the feature struct

  if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  // Load extension function pointers
  pfn_vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(m_device, "vkCmdBeginRenderingKHR");
  pfn_vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(m_device, "vkCmdEndRenderingKHR");

  // After creating the device, get the handles to the actual queues.
  vkGetDeviceQueue(m_device, indices.graphics_family, 0, &m_graphics_queue);
  vkGetDeviceQueue(m_device, indices.present_family, 0, &m_present_queue);
}

void VulkanDevice::createCommandPool() {
  QueueFamilyIndices queue_family_indices =
      findQueueFamilies(m_physical_device, m_surface);

  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = queue_family_indices.graphics_family;

  // These flags allow command buffers to be individually reset and are
  // allocated for short-lived submission. This is typical for command buffers
  // that are re-recorded every frame.
  pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

uint32_t VulkanDevice::findMemoryType(uint32_t type_filter,
                                      VkMemoryPropertyFlags properties) {
  // Query the physical device for its memory properties (types and heaps).
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_properties);

  // Iterate through the available memory types.
  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    // Check if the i-th bit is set in the type_filter (meaning this memory type
    // is allowed for the resource) AND if this memory type has all the
    // properties we require.
    if ((type_filter & (1 << i)) &&
        (mem_properties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i; // Found a suitable type, return its index.
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

// These public utility methods were missing from the source file, they need an
// implementation. I will add stub implementations for now.

SwapChainSupportDetails VulkanDevice::getSwapChainSupport() {
  return querySwapChainSupport(m_physical_device);
}

QueueFamilyIndices VulkanDevice::findPhysicalQueueFamilies() {
  return findQueueFamilies(m_physical_device, m_surface);
}

VkFormat
VulkanDevice::findSupportedFormat(const std::vector<VkFormat> &candidates,
                                  VkImageTiling tiling,
                                  VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

VkCommandBuffer VulkanDevice::beginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = m_command_pool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(m_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(m_graphics_queue);

  vkFreeCommandBuffers(m_device, m_command_pool, 1, &commandBuffer);
}

void VulkanDevice::copyBufferToImage(VkBuffer buffer, VkImage image,
                                     uint32_t width, uint32_t height,
                                     uint32_t layerCount) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = layerCount;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  endSingleTimeCommands(commandBuffer);
}

void VulkanDevice::transitionImageLayout(VkImage image, VkFormat format,
                                         VkImageLayout oldLayout,
                                         VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  endSingleTimeCommands(commandBuffer);
}

void VulkanDevice::createImage(const VkImageCreateInfo &imageInfo,
                               VkMemoryPropertyFlags properties, VkImage &image,
                               VkDeviceMemory &imageMemory) {
  if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(m_device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  if (vkBindImageMemory(m_device, image, imageMemory, 0) != VK_SUCCESS) {
    throw std::runtime_error("failed to bind image memory!");
  }
}

VkImageView VulkanDevice::createImageView(VkImage image, VkFormat format, VkImageAspectFlagBits flags) {
  VkImageViewCreateInfo view_info{};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = flags;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  VkImageView image_view;
  if (vkCreateImageView(m_device, &view_info, nullptr, &image_view) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }

  return image_view;
}

void VulkanDevice::createTextureSampler(VkSampler &textureSampler) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(m_physical_device, &properties);

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  if (vkCreateSampler(m_device, &samplerInfo, nullptr, &textureSampler) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }
}

void VulkanDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer &buffer,
                                VkDeviceMemory &bufferMemory) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

// This was a private helper function in the header, but its implementation was
// missing.
SwapChainSupportDetails
VulkanDevice::querySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface,
                                            &details.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count,
                                       nullptr);
  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count,
                                         details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface,
                                            &present_mode_count, nullptr);
  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, m_surface, &present_mode_count, details.present_modes.data());
  }
  return details;
}

} // namespace Render::Vulkan
