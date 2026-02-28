#pragma once

#include "drawing_policy_registry.h"
#include "i_renderer.h"
#include "pipeline_config_registry.h"
// Includes and forward-declarations for owned objects
#include "render_graph_executor.h"
#include "render_types.h"
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

  void init(ImGuiContext *ctx) override;

  void renderFrame(const Core::SceneView &view,
                   ImDrawData *ui_draw_data) override;

  void destroy() override;

  Device &getRenderDeivce() override { return *m_rhi_device; };

private:
  // This class now owns the core device and resource manager
  BackendType m_backend_type = BackendType::Vulkan;
  std::unique_ptr<VulkanDevice> m_device;
  std::unique_ptr<VulkanRHIDevice> m_rhi_device;
  VulkanResourceManager m_resource_manager;
  PipelineConfigRegistry m_pl_registry;
  DrawingPolicyRegistry m_dp_registry;
  std::unique_ptr<DescriptorPool> m_imgui_descriptor_pool{};
  RID m_per_frame_ds_layout;

  // --- Frame and Swapchain Management ---
  std::unique_ptr<VulkanSwapChain> m_swap_chain;
  std::vector<std::unique_ptr<VulkanCommandList>> m_command_lists;
  uint32_t m_acquired_image_index = 0;

  // --- Per-Frame Resources (one set per swapchain frame) ---
  struct PerFrameResources {
    RID uniform_buffer;       // FrameUniforms buffer (RID in resource manager)
    RID descriptor_set_rid;   // RID of descriptor set in resource manager
    VkDescriptorSet descriptor_set; // Set 0 for camera/projection (cached)
  };
  std::vector<PerFrameResources> m_per_frame_resources;

  // --- Rendering Logic (Orchestration) ---
  std::unique_ptr<RenderGraphExecutor> m_executor;
  ImGuiContext *m_imgui_context = nullptr;
  // Private, API-dependent methods for frame lifecycle management
  void createSwapChain(); // Will be called during initialization
  void createPerFrameResources(); // Create per-frame uniform buffers and descriptor sets
  void updatePerFrameResources(const Core::SceneView &view); // Update per-frame uniforms
  void acquireNextImage();
  void submitCommands();
  void present();
};

} // namespace Render::Vulkan
