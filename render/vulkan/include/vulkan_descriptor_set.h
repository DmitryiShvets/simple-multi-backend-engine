#pragma once
#include "vulkan_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace Render::Vulkan {

	class DescriptorSetLayout {
	public:
		class Builder {
		public:
			Builder(VulkanDevice& lveDevice) : lveDevice{ lveDevice } {}

			Builder& addBinding(
				uint32_t binding,
				VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags,
				uint32_t count = 1);

			std::unique_ptr<DescriptorSetLayout> build() const;

		private:
			VulkanDevice& lveDevice;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		};

		DescriptorSetLayout(
			VulkanDevice& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~DescriptorSetLayout();
		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
		const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& getBindings() const { return bindings; }

	private:
		VulkanDevice& lveDevice;
		VkDescriptorSetLayout descriptorSetLayout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class DescriptorWriter;
	};

	class DescriptorPool {
	public:
		class Builder {
		public:
			Builder(VulkanDevice& lveDevice) : lveDevice{ lveDevice } {}

			Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder& setMaxSets(uint32_t count);
			std::unique_ptr<DescriptorPool> build() const;

		private:
			VulkanDevice& lveDevice;
			std::vector<VkDescriptorPoolSize> poolSizes{};
			uint32_t maxSets = 1000;
			VkDescriptorPoolCreateFlags poolFlags = 0;
		};

		DescriptorPool(
			VulkanDevice& lveDevice,
			uint32_t maxSets,
			VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize>& poolSizes);
		~DescriptorPool();
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;

		bool allocateDescriptor(
			const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

		void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

		void resetPool();
		VkDescriptorPool get_descriptor_pool() { return descriptorPool; };

	private:
		VulkanDevice& lveDevice;
		VkDescriptorPool descriptorPool;

		friend class DescriptorWriter;
	};

	class DescriptorWriter {
	public:
		DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

		DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		VkDescriptorSet build();
		void overwrite(VkDescriptorSet& set);

	private:
		DescriptorSetLayout& setLayout;
		DescriptorPool& pool;
		std::vector<VkWriteDescriptorSet> writes;
	};
}

// Справочник
// |-----|---------|-------|
// | `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER` | Uniform Buffer (UBO) | Константы для шейдера (матрицы, параметры) |
// | `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` | Storage Buffer (SSBO) | Чтение/запись больших данных |
// | `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` | Текстура + сэмплер | Изображения для шейдера |
// | `VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE` | Только текстура | Без сэмплера |
// | `VK_DESCRIPTOR_TYPE_SAMPLER` | Только сэмплер | Параметры фильтрации |
// | `VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT` | Subpass input | Для render pass |
// | `VK_DESCRIPTOR_TYPE_STORAGE_IMAGE` | Read/write image | Для compute шейдеров |
// | `VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER` | Uniform texel buffer | Структурированные данные |
// | `VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER` | Storage texel buffer | Read/write структурированные данные |
// ---
