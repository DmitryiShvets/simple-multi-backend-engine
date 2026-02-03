#pragma once

#include "rendering_device.h"
#include "simple_render_system.h"
#include "vulkan_resource_manager.h"
#include <memory>
#include <vector>

namespace Render::Vulkan {
// Forward declaration
class VulkanDevice;
class SimpleRenderSystem;
class VulkanSwapChain;
class DescriptorSetLayout;
class VulkanCommandList;

// This is the concrete implementation of the main RHI interface for Vulkan.
// The application will create an instance of this class.
class VulkanRenderingDevice : public RenderingDevice {
public:
  VulkanRenderingDevice(std::unique_ptr<VulkanDevice> device);
  virtual ~VulkanRenderingDevice() override;

  // --- RenderingDevice Interface Implementation ---
  RID createSwapChain(const SwapChainDesc &desc) override;
  void free(RID rid) override;
  void tick() override;
  CommandList *beginCommandList(RID swapChain) override;
  void submitCommandList(RID swapChain, CommandList *list) override;

  // Методы для управления циклом кадра
  RID acquireNextFrame(RID swapChain) override;
  void present(RID swapChain) override;
  void waitIdle() override;

private:
  // The rendering device owns the low-level device.
  std::unique_ptr<VulkanDevice> m_device;

  std::unique_ptr<VulkanSwapChain> m_swap_chain;

  // It also owns the resource manager.
  VulkanResourceManager m_resource_manager;

  // --- Command Management ---
  // A pool of command lists, one for each frame in flight.
  // We use unique_ptr because VulkanCommandList is non-copyable.
  std::vector<std::unique_ptr<VulkanCommandList>> m_command_lists;
  uint32_t m_acquired_image_index = 0;

  std::unique_ptr<SimpleRenderSystem> m_test_rs;
  std::unique_ptr<DescriptorSetLayout> m_set_layout;
};

} // namespace Render::Vulkan
