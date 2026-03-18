#include "vulkan_device.h"
#include "platform.h"
#include "utils/logger.h"

#include "vulkan_helpers.h"
#include "vulkan_types.h"
#include <algorithm>
#include <cstring>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace ssme::vulkan {
/**
 * @brief This is the callback function that Vulkan's validation layers will
 * call. Whenever a validation rule is broken, this function is invoked with
 * details about the error.
 */
static VKAPI_ATTR vk::Bool32 VKAPI_CALL
debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
              vk::DebugUtilsMessageTypeFlagsEXT message_type,
              const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data,
              void *p_user_data) {
  Logger::validation_log(vk::to_string(message_type),
                         p_callback_data->pMessage);
  return vk::False; // Should always return VK_FALSE
}

bool checkDeviceFeaturesSupport(const vk::raii::PhysicalDevice &device);
// --- VulkanDevice Method Implementations ---

VulkanDevice::VulkanDevice(ssme::Platform *platform) : m_platform(platform) {
  initialize();
}

VulkanDevice::~VulkanDevice() {}

void VulkanDevice::initialize() {
  // This is the main entry point that orchestrates the entire setup process.
  // 1. Get required extensions from the surface creator (e.g., for GLFW).
  auto extensions = m_platform->getRequiredVulkanInstanceExtensions();
  std::vector<const char *> layers;
  // 2. In debug builds, add the extension needed for the debug messenger.
  if (g_enable_validation_layers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
    layers.assign(m_validation_layers.begin(), m_validation_layers.end());
  }

  // 3. Create the Vulkan Instance and setup validation layers.
  createInstance(extensions, layers);
  // 3.a After creating the instance, we also need to create the messenger
  // object itself.
  setupDebugMessenger();
  // 4. Create the window surface using the strategy object.
  createSurface();
  // 5. Select a physical GPU.
  pickPhysicalDevice();
  // 6. Create a logical device to interface with the GPU.
  createLogicalDevice();
  // 7. Create a pool for command buffers.
  createCommandPool();
}

void VulkanDevice::createInstance(
    const std::vector<const char *> &required_extensions,
    const std::vector<const char *> &required_layers) {
  // --- Check for Validation Layer support in debug builds ---
  auto available_layers = m_context.enumerateInstanceLayerProperties();
  for (const auto &layer : required_layers) {
    if (std::ranges::none_of(available_layers, [&layer](const auto &avail) {
          return strcmp(avail.layerName, layer) == 0;
        })) {
      throw std::runtime_error("Required validation layer is not available: " +
                               std::string(layer));
    }
  }
  // --- Check if the required extensions are supported ---
  auto available_extensions = m_context.enumerateInstanceExtensionProperties();
  for (const auto &extension : required_extensions) {
    if (std::ranges::none_of(
            available_extensions, [&extension](const auto &avail) {
              return strcmp(avail.extensionName, extension) == 0;
            })) {
      throw std::runtime_error("Required extension not supported: " +
                               std::string(extension));
    }
  }
  // --- Fill out Application Info ---
  // This struct is optional but provides good information to the driver.
  constexpr vk::ApplicationInfo app_info{
      .pApplicationName = "Primitives App",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion14};
  // --- Fill out Instance Create Info ---
  // This is the main struct for creating the instance.
  vk::InstanceCreateInfo create_info{
      .pApplicationInfo = &app_info,
      .enabledLayerCount = static_cast<uint32_t>(required_layers.size()),
      .ppEnabledLayerNames = required_layers.data(),
      .enabledExtensionCount =
          static_cast<uint32_t>(required_extensions.size()),
      .ppEnabledExtensionNames = required_extensions.data(),
  };

  // Finally, create the instance!
  m_instance = vk::raii::Instance(m_context, create_info);
}

void VulkanDevice::setupDebugMessenger() {
  if (!g_enable_validation_layers)
    return;
  // This method creates the debug messenger object.
  vk::DebugUtilsMessageSeverityFlagsEXT severity_flags(
      // vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT message_type(
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);
  vk::DebugUtilsMessengerCreateInfoEXT create_info{
      .messageSeverity = severity_flags,
      .messageType = message_type,
      .pfnUserCallback = &debugCallback};
  m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(create_info);
}

void VulkanDevice::createSurface() {
  VkSurfaceKHR _surface =
      (VkSurfaceKHR)m_platform->createVulkanSurface(*m_instance);
  if (!_surface) {
    throw std::runtime_error("Failed to create window surface!");
  }
  m_surface = vk::raii::SurfaceKHR(m_instance, _surface);
}

void VulkanDevice::pickPhysicalDevice() {
  // Get the list of all available GPUs with Vulkan support.
  std::vector devices = m_instance.enumeratePhysicalDevices();
  // Iterate through the GPUs and pick the first one that is suitable.
  // A more advanced implementation could score devices and pick the best one
  // (e.g., prefer discrete GPUs).
  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
      m_physical_device = device;
      break;
    }
  }

  if (m_physical_device == nullptr) {
    throw std::runtime_error("Failed to find a suitable GPU!");
  }
}

bool VulkanDevice::isDeviceSuitable(const vk::raii::PhysicalDevice &device) {
  // A device is suitable if it supports the queue families and extensions we
  // need.
  bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_3;
  isSuitable = isSuitable && findQueueFamilies(device, *m_surface).isComplete();
  isSuitable = isSuitable && checkDeviceExtensionSupport(device);
  isSuitable = isSuitable && checkDeviceFeaturesSupport(device);
  // It must support both graphics/present queues and the swapchain extension.
  // A full implementation would also check for swapchain adequacy (e.g., at
  // least one format and present mode available).
  return isSuitable;
}

bool checkDeviceFeaturesSupport(const vk::raii::PhysicalDevice &device) {
  auto features = device.template getFeatures2<
      vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

  const auto &core_features =
      features.template get<vk::PhysicalDeviceFeatures2>().features;
  const auto &vulkan13_features =
      features.template get<vk::PhysicalDeviceVulkan13Features>();
  const auto &ext_features =
      features
          .template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

  bool supports_features =
      core_features.samplerAnisotropy && vulkan13_features.dynamicRendering &&
      vulkan13_features.synchronization2 && ext_features.extendedDynamicState;

  return supports_features;
}

// Check if the device supports all the extensions we require
bool VulkanDevice::checkDeviceExtensionSupport(
    const vk::raii::PhysicalDevice &device) {
  auto available_extensions = device.enumerateDeviceExtensionProperties();
  std::set<std::string> required_extensions(m_device_extensions.begin(),
                                            m_device_extensions.end());
  for (const auto &extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

QueueFamilyIndices
VulkanDevice::findQueueFamilies(const vk::raii::PhysicalDevice &device,
                                vk::SurfaceKHR surface) {
  // Find queue families that support graphics operations and presentation to
  // our surface.
  QueueFamilyIndices indices;
  auto queue_families = device.getQueueFamilyProperties();
  int i = 0;
  for (const auto &queue_family : queue_families) {
    // Check for graphics support
    if (queue_family.queueCount > 0 &&
        queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphics_family = i;
      indices.graphics_family_has_value = true;
    }

    // Check for presentation support to the window surface
    vk::Bool32 present_support = device.getSurfaceSupportKHR(i, surface);
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
  m_queue_family_indices = findQueueFamilies(m_physical_device, m_surface);

  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
  std::set unique_queue_families = {m_queue_family_indices.graphics_family,
                                    m_queue_family_indices.present_family};

  float queue_priority = 1.0f;
  for (auto queue_family : unique_queue_families) {
    vk::DeviceQueueCreateInfo create_info{
        .queueFamilyIndex = queue_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };
    queue_create_infos.push_back(create_info);
  }
  // --- Enable Dynamic Rendering Feature ---
  // query for Vulkan 1.3 features
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      feature_chain = {vk::PhysicalDeviceFeatures2{
                           .features = {.samplerAnisotropy = vk::True}},
                       {
                           .synchronization2 = vk::True,
                           .dynamicRendering = vk::True,
                       },
                       {.extendedDynamicState = vk::True}};
  vk::DeviceCreateInfo create_info{
      .pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
      .pQueueCreateInfos = queue_create_infos.data(),
      .enabledExtensionCount =
          static_cast<uint32_t>(m_device_extensions.size()),
      .ppEnabledExtensionNames = m_device_extensions.data(),
  };

  m_device = vk::raii::Device(m_physical_device, create_info);

  // After creating the device, get the handles to the actual queues.
  m_graphics_queue =
      vk::raii::Queue(m_device, m_queue_family_indices.graphics_family, 0);
  m_present_queue =
      vk::raii::Queue(m_device, m_queue_family_indices.present_family, 0);
}

void VulkanDevice::createCommandPool() {
  // These flags allow command buffers to be individually reset and are
  // allocated for short-lived submission. This is typical for command buffers
  // that are re-recorded every frame.
  vk::CommandPoolCreateInfo create_info{
      .flags = (vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
                vk::CommandPoolCreateFlagBits::eTransient),
      .queueFamilyIndex = m_queue_family_indices.graphics_family};

  m_command_pool = vk::raii::CommandPool(m_device, create_info);
}

uint32_t VulkanDevice::findMemoryType(uint32_t type_filter,
                                      vk::MemoryPropertyFlags properties) {
  // Query the physical device for its memory properties (types and heaps).
  vk::PhysicalDeviceMemoryProperties mem_properties =
      m_physical_device.getMemoryProperties();

  // Iterate through the available memory types.
  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    // Check if the i-th bit is set in the type_filter (meaning this memory
    // type is allowed for the resource) AND if this memory type has all the
    // properties we require.
    if ((type_filter & (1 << i)) &&
        (mem_properties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i; // Found a suitable type, return its index.
    }
  }
  throw std::runtime_error("Failed to find suitable memory type!");
}

// These public utility methods were missing from the source file, they need
// an implementation. I will add stub implementations for now.

SwapChainSupportDetails VulkanDevice::getSwapChainSupport() {
  return querySwapChainSupport(m_physical_device);
}

QueueFamilyIndices VulkanDevice::getPhysicalQueueFamilies() {
  return m_queue_family_indices;
}

vk::Format
VulkanDevice::findSupportedFormat(const std::vector<vk::Format> &candidates,
                                  vk::ImageTiling tiling,
                                  vk::FormatFeatureFlags features) {
  for (vk::Format format : candidates) {
    vk::FormatProperties props = m_physical_device.getFormatProperties(format);
    if (tiling == vk::ImageTiling::eLinear &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == vk::ImageTiling::eOptimal &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("Failed to find supported format!");
}

vk::raii::CommandBuffer VulkanDevice::createCommandBuffer() const {
  vk::CommandBufferAllocateInfo create_info{
      .commandPool = m_command_pool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1};

  auto command_buffer =
      std::move(vk::raii::CommandBuffers(m_device, create_info).front());
  return command_buffer;
}

vk::raii::CommandBuffer VulkanDevice::beginSingleTimeCommands() {
  vk::raii::CommandBuffer command_buffer = createCommandBuffer();
  vk::CommandBufferBeginInfo begin_info{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  command_buffer.begin(begin_info);
  return command_buffer;
}

void VulkanDevice::endSingleTimeCommands(
    vk::raii::CommandBuffer command_buffer) {
  command_buffer.end();
  vk::SubmitInfo submit_info{
      .commandBufferCount = 1,
      .pCommandBuffers = &*command_buffer,
  };
  m_graphics_queue.submit(submit_info);
  m_graphics_queue.waitIdle();
}

void VulkanDevice::copyBufferToImage(vk::Buffer buffer, vk::Image image,
                                     uint32_t width, uint32_t height,
                                     uint32_t layerCount) {
  vk::raii::CommandBuffer command_buffer = beginSingleTimeCommands();

  vk::BufferImageCopy region{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource =
          vk::ImageSubresourceLayers{.aspectMask =
                                         vk::ImageAspectFlagBits::eColor,
                                     .mipLevel = 0,
                                     .baseArrayLayer = 0,
                                     .layerCount = layerCount},
      .imageOffset = vk::Offset3D{.x = 0, .y = 0, .z = 0},
      .imageExtent =
          vk::Extent3D{.width = width, .height = height, .depth = 1}};
  command_buffer.copyBufferToImage(
      buffer, image, vk::ImageLayout::eTransferDstOptimal, {region});
  endSingleTimeCommands(std::move(command_buffer));
}

void VulkanDevice::pipelineBarrier(const vk::raii::CommandBuffer &cmd_buffer,
                                   vk::Image image, vk::Format format,
                                   vk::ImageLayout old_layout,
                                   vk::ImageLayout new_layout,
                                   vk::AccessFlags2 src_access_mask,
                                   vk::AccessFlags2 dst_access_mask,
                                   vk::PipelineStageFlags2 src_stage_mask,
                                   vk::PipelineStageFlags2 dst_stage_mask) {

  vk::ImageAspectFlagBits aspectMask = isDepthFormat(format)
                                           ? vk::ImageAspectFlagBits::eDepth
                                           : vk::ImageAspectFlagBits::eColor;
  vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask = src_stage_mask,
      .srcAccessMask = src_access_mask,
      .dstStageMask = dst_stage_mask,
      .dstAccessMask = dst_access_mask,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {.aspectMask = aspectMask,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  vk::DependencyInfo dependency_info = {.dependencyFlags = {},
                                        .imageMemoryBarrierCount = 1,
                                        .pImageMemoryBarriers = &barrier};
  cmd_buffer.pipelineBarrier2(dependency_info);
}

void VulkanDevice::transitionImageLayout(vk::Image image, vk::Format format,
                                         vk::ImageLayout old_layout,
                                         vk::ImageLayout new_layout) {
  vk::raii::CommandBuffer command_buffer = beginSingleTimeCommands();
  auto masks = getImageMemoryBarrierMasks(old_layout, new_layout);
  pipelineBarrier(command_buffer, image, format, old_layout, new_layout,
                  masks.src_mask, masks.dst_mask, masks.src_stages,
                  masks.dst_stages);
  endSingleTimeCommands(std::move(command_buffer));
}

ImageResource VulkanDevice::createImage(const vk::ImageCreateInfo &imageInfo,
                                        vk::MemoryPropertyFlags properties) {
  vk::raii::Image image = vk::raii::Image(m_device, imageInfo);
  vk::MemoryRequirements mem_requirements = image.getMemoryRequirements();

  auto index = findMemoryType(mem_requirements.memoryTypeBits, properties);
  vk::MemoryAllocateInfo alloc_info{.allocationSize = mem_requirements.size,
                                    .memoryTypeIndex = index};

  vk::raii::DeviceMemory image_memory =
      vk::raii::DeviceMemory(m_device, alloc_info);
  image.bindMemory(image_memory, 0);

  return {std::move(image), std::move(image_memory)};
}

vk::raii::ImageView
VulkanDevice::createImageView(vk::Image image, vk::Format format,
                              vk::ImageAspectFlagBits flags) {
  vk::ImageViewCreateInfo create_info{
      .flags = {},
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .components = {},
      .subresourceRange = vk::ImageSubresourceRange{.aspectMask = flags,
                                                    .baseMipLevel = 0,
                                                    .levelCount = 1,
                                                    .baseArrayLayer = 0,
                                                    .layerCount = 1}

  };

  vk::raii::ImageView image_view{m_device, create_info};
  return std::move(image_view);
}

vk::raii::Sampler VulkanDevice::createTextureSampler() {
  vk::PhysicalDeviceProperties properties = m_physical_device.getProperties();
  vk::SamplerCreateInfo sampler_info{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::True,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways,
      .borderColor = vk::BorderColor::eIntOpaqueBlack,
      .unnormalizedCoordinates = vk::False,
  };
  vk::raii::Sampler sampler = vk::raii::Sampler(m_device, sampler_info);
  return sampler;
}

BufferResource VulkanDevice::createBuffer(vk::DeviceSize size,
                                          vk::BufferUsageFlags usage,
                                          vk::MemoryPropertyFlags properties) {
  vk::BufferCreateInfo buffer_info{
      .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
  vk::raii::Buffer buffer = vk::raii::Buffer(m_device, buffer_info);

  vk::MemoryRequirements mem_requirements = buffer.getMemoryRequirements();

  auto index = findMemoryType(mem_requirements.memoryTypeBits, properties);
  vk::MemoryAllocateInfo alloc_info{.allocationSize = mem_requirements.size,
                                    .memoryTypeIndex = index};

  vk::raii::DeviceMemory buffer_memory =
      vk::raii::DeviceMemory(m_device, alloc_info);
  buffer.bindMemory(buffer_memory, 0);

  return {std::move(buffer), std::move(buffer_memory)};
}

SwapChainSupportDetails
VulkanDevice::querySwapChainSupport(const vk::raii::PhysicalDevice &device) {
  SwapChainSupportDetails details;
  details.capabilities = device.getSurfaceCapabilitiesKHR(m_surface);
  details.formats = device.getSurfaceFormatsKHR(m_surface);
  details.present_modes = device.getSurfacePresentModesKHR(m_surface);
  return details;
}

} // namespace ssme::vulkan
