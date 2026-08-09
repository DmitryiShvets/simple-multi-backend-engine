#pragma once

#include "core/gpu_types.h"
#include "renderer.h"
#include "vulkan_gpu_storage.h"
#include "vulkan_render_device.h"
#include <vulkan/vulkan.hpp> // todo remove it with  vk::DescriptorSet descriptor_set
#include <memory>
#include <vector>

namespace ssme::vulkan {
class VulkanDevice;
class VulkanSwapChain;
class VulkanDescriptorSetLayout;
class VulkanCommandList;
} // namespace ssme::vulkan

namespace ssme {

class RenderDevice;
class Platform;
class ResourceManager;
class SceneView;

// This is the concrete, API-dependent implementation of the IRenderer interface
// for Vulkan. It acts as the main orchestrator for the Vulkan backend (Level
// 3). It owns all top-level Vulkan objects and the shared, API-agnostic
// rendering systems.
class VulkanRenderer final : public IRenderer {
public:
  VulkanRenderer(Platform *platform,  ResourceManager* rm );
  virtual ~VulkanRenderer();

  void init(ImGuiContext *ctx) override;

  void renderFrame(SceneView &view, ImDrawData *ui_draw_data) override;

  void destroy() override;

  void waitIdle() const override;

  RenderDevice &getRenderDeivce() override { return *m_rhi_device; };

  GpuBackend getGpuBackend() override;

  void setFrameResources(std::shared_ptr<FrameData> data) override;

bool supportsFrameGraph() const override { return true; }
void beginFrame() override;
SwapchainInfo getSwapchain() override;
uint32_t getCurrentFrameIndex() const override;
void renderImGui(CommandList &, ImDrawData *) override;
void renderFrameGraph(RenderGraph &, const CompiledPlan &, SceneView &, ImDrawData *) override;

private:
  GpuBackend m_backend_type = GpuBackend::Vulkan;
  Platform * m_platform;
  ResourceManager* m_rm;
  std::unique_ptr<ssme::vulkan::VulkanDevice> m_device;
  std::unique_ptr<ssme::vulkan::VulkanRenderDevice> m_rhi_device;
  vulkan::VulkanGpuStorageMT m_storage;
  std::unique_ptr<ssme::vulkan::VulkanDescriptorPool> m_imgui_descriptor_pool{};
  RID m_per_frame_ds_layout;

  // --- Frame and Swapchain Management ---
  std::unique_ptr<ssme::vulkan::VulkanSwapChain> m_swap_chain;
  std::vector<std::unique_ptr<ssme::vulkan::VulkanCommandList>> m_command_lists;
  uint32_t m_acquired_image_index = 0;

  std::shared_ptr<FrameData> m_frame_data = nullptr;

  // --- Rendering Logic (Orchestration) ---
  ImGuiContext *m_imgui_context = nullptr;
  // Private, API-dependent methods for frame lifecycle management
  void
  updatePerFrameResources(const SceneView &view); // Update per-frame uniforms
  void acquireNextImage(); // wait for fences and retrives new image
  void submitCommands();
  void present();
};

} // namespace ssme
