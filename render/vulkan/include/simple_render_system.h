#pragma once
#include "vulkan_pipeline.h"
#include <memory>
namespace Render::Vulkan {

	class SimpleRenderSystem {
	public:

		SimpleRenderSystem(VulkanDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout descriptor_layout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void render(VkCommandBuffer command_buffer);

	private:
		void create_pipline_layout(VkDescriptorSetLayout descriptor_layout);
		void create_pipline(VkRenderPass render_pass);

		VulkanDevice& m_device;

		std::unique_ptr<VulkanPipeLine> m_pipeline;
		VkPipelineLayout m_pipeline_layout;
	};

}
