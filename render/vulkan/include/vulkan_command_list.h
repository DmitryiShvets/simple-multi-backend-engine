#pragma once

#include "command_list.h"
#include "render_types.h"
#include "vulkan_device.h"
#include "vulkan_resource_manager.h"
#include <vulkan/vulkan.h>

namespace Render::Vulkan {

class VulkanCommandList : public CommandList {
public:
  VulkanCommandList(VulkanDevice &vkDevice,
                    VulkanResourceManager &vkResourceManager);
  ~VulkanCommandList();

  // Prevent copying and assignment, as this class manages a unique resource.
  VulkanCommandList(const VulkanCommandList &) = delete;
  VulkanCommandList &operator=(const VulkanCommandList &) = delete;

  void begin() override;
  void end() override;
  void clearRenderTarget(RID renderTarget, const float color[4]) override;
  void resourceBarrier(RID resource, ResourceState before,
                       ResourceState after) override;
  VkCommandBuffer getHandle() const { return m_command_buffer; }
private:
  VulkanDevice &m_device;
  VulkanResourceManager &m_resource_manager;
  VkCommandBuffer m_command_buffer;
};

} // namespace Render::Vulkan
