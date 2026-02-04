#pragma once

#include "render_device.h"
#include "vulkan_resource_manager.h"
#include <memory>

namespace Render::Vulkan {

class VulkanDevice;

// This class is the concrete Vulkan implementation of the pure Device interface (Level 5).
// Its name is changed to reflect its role.
class VulkanRHIDevice final : public Device {
public:
  VulkanRHIDevice(std::unique_ptr<VulkanDevice> device);
  virtual ~VulkanRHIDevice() override;

  // --- Device Interface Implementation ---
  void free(RID rid) override;
  void tick() override;
  void waitIdle() override;

  // ...

private:
  std::unique_ptr<VulkanDevice> m_device;

  // ...and the resource manager.
  VulkanResourceManager m_resource_manager;
};

} // namespace Render::Vulkan
