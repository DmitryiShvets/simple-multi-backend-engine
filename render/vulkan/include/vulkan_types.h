#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace Render::Vulkan {

struct ImageBarrierMask2 {
  vk::AccessFlags2 src_mask;
  vk::AccessFlags2 dst_mask;
  vk::PipelineStageFlags2 src_stages;
  vk::PipelineStageFlags2 dst_stages;
};
// required to create swap chain
struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> present_modes;
};

// required to create physical device. also need to cteate swap chain
struct QueueFamilyIndices {
  uint32_t graphics_family;
  uint32_t present_family;
  bool graphics_family_has_value = false;
  bool present_family_has_value = false;
  bool isComplete() {
    return graphics_family_has_value && present_family_has_value;
  }
};

struct BufferResource {
  vk::raii::Buffer buffer;
  vk::raii::DeviceMemory memory;
};

struct ImageResource {
  vk::raii::Image image;
  vk::raii::DeviceMemory memory;
};

inline ImageBarrierMask2
getImageMemoryBarrierMasks(vk::ImageLayout old_layout,
                           vk::ImageLayout new_layout) {
  ImageBarrierMask2 data;
  // supported cases
  if (old_layout == vk::ImageLayout::eUndefined &&
      new_layout == vk::ImageLayout::eTransferDstOptimal) {
    data.src_mask = {};
    data.dst_mask = vk::AccessFlagBits2::eTransferWrite;
    data.src_stages = vk::PipelineStageFlagBits2::eTopOfPipe;
    data.dst_stages = vk::PipelineStageFlagBits2::eTransfer;
  } else if (old_layout == vk::ImageLayout::eTransferDstOptimal &&
             new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    data.src_mask = vk::AccessFlagBits2::eTransferWrite;
    data.dst_mask = vk::AccessFlagBits2::eShaderRead;
    data.src_stages = vk::PipelineStageFlagBits2::eTransfer;
    data.dst_stages = vk::PipelineStageFlagBits2::eFragmentShader;
  } else if (old_layout == vk::ImageLayout::eUndefined &&
             new_layout == vk::ImageLayout::eDepthAttachmentOptimal) {
    data.src_mask = {};
    data.dst_mask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
    data.src_stages = vk::PipelineStageFlagBits2::eTopOfPipe;
    data.dst_stages = vk::PipelineStageFlagBits2::eEarlyFragmentTests;
  } else if (old_layout == vk::ImageLayout::eUndefined &&
             new_layout == vk::ImageLayout::eColorAttachmentOptimal) {
    data.src_mask = {};
    data.dst_mask = vk::AccessFlagBits2::eColorAttachmentWrite;
    data.src_stages = vk::PipelineStageFlagBits2::eTopOfPipe;
    data.dst_stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
  } else if (old_layout == vk::ImageLayout::eColorAttachmentOptimal &&
             new_layout == vk::ImageLayout::ePresentSrcKHR) {
    data.src_mask = vk::AccessFlagBits2::eColorAttachmentWrite;
    data.dst_mask = {};
    data.src_stages = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    data.dst_stages = vk::PipelineStageFlagBits2::eBottomOfPipe;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }
  return data;
}
} // namespace Render::Vulkan
