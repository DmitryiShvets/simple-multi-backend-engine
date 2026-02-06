#pragma once

#include "i_renderer.h"

// Includes and forward-declarations for owned objects
#include "vulkan_resource_manager.h"
#include <memory>
#include <vector>

namespace Render::Vulkan {
    class VulkanDevice;
    class VulkanSwapChain;
    class SimpleRenderSystem;
    class DescriptorSetLayout;
    class VulkanCommandList;
    class VulkanDataBuffer; // new
    class VulkanTexture;   // new
    class DescriptorPool;  // new
}
namespace Render {
    class Device;
}
class SceneView; // Defined in a higher-level header

namespace Render::Vulkan {

// This is the concrete, API-dependent implementation of the IRenderer interface for Vulkan.
// It acts as the main orchestrator for the Vulkan backend (Level 3).
// It owns all top-level Vulkan objects and the shared, API-agnostic rendering systems.
class VulkanRenderer final : public IRenderer {
public:
    VulkanRenderer(std::unique_ptr<VulkanDevice> device);
    virtual ~VulkanRenderer();

    // The implementation of the main interface method
    void renderFrame(const SceneView& view) override;

private:
    // This class now owns the core device and resource manager
    std::unique_ptr<VulkanDevice> m_device;
    VulkanResourceManager m_resource_manager;

    // --- Frame and Swapchain Management ---
    std::unique_ptr<VulkanSwapChain> m_swap_chain;
    std::vector<std::unique_ptr<VulkanCommandList>> m_command_lists;
    uint32_t m_acquired_image_index = 0;

    // --- Rendering Logic (Orchestration) ---
    // In the future, this will be replaced by SceneRenderer and RenderGraphExecutor
    std::unique_ptr<SimpleRenderSystem> m_test_rs;
    std::unique_ptr<DescriptorSetLayout> m_set_layout;

    // --- Resource Management Demo ---
    std::unique_ptr<DescriptorPool> m_global_pool;
    VkDescriptorSet m_global_descriptor_set;
    std::unique_ptr<VulkanDataBuffer> m_vertex_buffer;
    std::unique_ptr<VulkanTexture> m_texture;


    // Private, API-dependent methods for frame lifecycle management
    void createSwapChain(); // Will be called during initialization
    void acquireNextImage();
    void submitCommands();
    void present();
};

} // namespace Render::Vulkan
