#ifndef MAUREN_VULKANDESCRIPTORCONTEXT_H
#define MAUREN_VULKANDESCRIPTORCONTEXT_H

#include "RendererPCH.h"
#include "VulkanBuffer.h"

#include "Assets/BindlessData.h"

namespace MauRen
{
	// Currently only supports one layout
	class VulkanDescriptorContext final
	{
	public:
		explicit VulkanDescriptorContext() = default;
		~VulkanDescriptorContext() = default;

		void Initialize() {}
		void Destroy();

		[[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const noexcept { return m_DescriptorSetLayout; }
		[[nodiscard]] std::vector<VkDescriptorSet> const& GetDescriptorSets() const noexcept { return m_DescriptorSets; }
		[[nodiscard]] VkDescriptorPool GetDescriptorPool() const noexcept { return m_DescriptorPool; }

		void BindTexture(uint32_t destLocation, VkImageView imageView, VkImageLayout imageLayout);
		void BindMaterialBuffer(VkDescriptorBufferInfo bufferInfo, uint32_t frame);

		void BindShadowMap(uint32_t destLocation, VkImageView imageView, VkImageLayout imageLayout);

		void CreateDescriptorSetLayout();
		void CreateDescriptorPool();
		void CreateDescriptorSets(std::vector<VulkanBuffer> const& bufferInfoBuffers, VkDeviceSize offset, VkDeviceSize range, VkImageLayout imageLayout, std::vector<VkImageView> const& imageViews = {}, VkSampler sampler = {});

		VulkanDescriptorContext(VulkanDescriptorContext const&) = delete;
		VulkanDescriptorContext(VulkanDescriptorContext&&) = delete;
		VulkanDescriptorContext& operator=(VulkanDescriptorContext const&) = delete;
		VulkanDescriptorContext& operator=(VulkanDescriptorContext&&) = delete;

	private:
		VkDescriptorSetLayout m_DescriptorSetLayout{ VK_NULL_HANDLE };
		VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };

		// !
		// common practice to rank from "least updated" descriptor set(index 0) to "most frequent updated"
		// descriptor set(index N)!This way, we can avoid rebinding as much as possible!
		std::vector<VkDescriptorSet> m_DescriptorSets{};

		uint32_t const UBO_BINDING_SLOT{ 0 };
		uint32_t const SAMPLER_BINDING_SLOT{ 1 };
		uint32_t const TEXTURE_BINDING_SLOT{ 2 };
		uint32_t const MATERIAL_DATA_BINDING_SLOT{ 3 };

		uint32_t const MESH_DATA_BINDING_SLOT{ 4 };
		uint32_t const MESH_INSTANCE_DATA_BINDING_SLOT{ 5 };

		uint32_t const GBUFFER_COLOR_SLOT{ 6 };
		uint32_t const GBUFFER_NORMAL_SLOT{ 7 };
		uint32_t const GBUFFER_METAL_SLOT{ 8 };
		uint32_t const GBUFFER_DEPTH_SLOT{ 9 };

		uint32_t const HDRI_COLOR_SLOT{ 10 };

		uint32_t const SHADOW_MAPS_SLOT{ 11 };
		//TODO Shadow Sampler
	};
}

#endif