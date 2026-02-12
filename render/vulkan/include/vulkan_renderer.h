#pragma once

#include "i_renderer.h"

// Includes and forward-declarations for owned objects
#include "render_graph_executor.h"
#include "vulkan_resource_manager.h"
#include "vulkan_rhi_device.h"
#include <memory>
#include <vector>

namespace Render::Vulkan {
class VulkanDevice;
class VulkanSwapChain;
class DescriptorSetLayout;
class VulkanCommandList;
} // namespace Render::Vulkan
namespace Render {
class Device;
}
namespace Core {
class SceneView;
}

namespace Render::Vulkan {

// This is the concrete, API-dependent implementation of the IRenderer interface
// for Vulkan. It acts as the main orchestrator for the Vulkan backend (Level
// 3). It owns all top-level Vulkan objects and the shared, API-agnostic
// rendering systems.
class VulkanRenderer final : public IRenderer {
public:
  VulkanRenderer(std::unique_ptr<VulkanDevice> device);
  virtual ~VulkanRenderer();

  // The implementation of the main interface method
  void renderFrame(const Core::SceneView &view) override;
  Device &getRenderDeivce() override { return *m_rhi_device; };

private:
  // This class now owns the core device and resource manager
  std::unique_ptr<VulkanDevice> m_device;
  std::unique_ptr<VulkanRHIDevice> m_rhi_device;
  VulkanResourceManager m_resource_manager;

  // --- Frame and Swapchain Management ---
  std::unique_ptr<VulkanSwapChain> m_swap_chain;
  std::vector<std::unique_ptr<VulkanCommandList>> m_command_lists;
  uint32_t m_acquired_image_index = 0;

  // --- Rendering Logic (Orchestration) ---
  std::unique_ptr<RenderGraphExecutor> m_executor;

  // Private, API-dependent methods for frame lifecycle management
  void createSwapChain(); // Will be called during initialization
  void acquireNextImage();
  void submitCommands();
  void present();
};

} // namespace Render::Vulkan
