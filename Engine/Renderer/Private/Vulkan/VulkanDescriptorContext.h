#ifndef MAUREN_VULKANDESCRIPTORCONTEXT_H
#define MAUREN_VULKANDESCRIPTORCONTEXT_H

#include "RendererPCH.h"
#include "VulkanBuffer.h"

namespace MauRen
{
	// Currently only supports one layout
	class VulkanDescriptorContext final
	{
	public:
		explicit VulkanDescriptorContext() = default;
		~VulkanDescriptorContext() = default;

		void Initialize(){}
		void Destroy();

		[[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const noexcept { return m_DescriptorSetLayout; }
		[[nodiscard]] std::vector<VkDescriptorSet> const& GetDescriptorSets() const noexcept { return m_DescriptorSets; }
		[[nodiscard]] VkDescriptorPool GetDescriptorPool() const noexcept { return m_DescriptorPool; }

		void AddTexture(VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout);

		void CreateDescriptorSetLayout();
		void CreateDescriptorPool();
		void CreateDescriptorSets(std::vector<VulkanBuffer> const& bufferInfoBuffers, VkDeviceSize offset, VkDeviceSize range, VkImageLayout imageLayout, std::vector<VkImageView> const& imageViews, VkSampler sampler);

		VulkanDescriptorContext(VulkanDescriptorContext const&) = delete;
		VulkanDescriptorContext(VulkanDescriptorContext&&) = delete;
		VulkanDescriptorContext& operator=(VulkanDescriptorContext const&) = delete;
		VulkanDescriptorContext& operator=(VulkanDescriptorContext&&) = delete;

	private:
		VkDescriptorSetLayout m_DescriptorSetLayout{ VK_NULL_HANDLE };
		VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };
		uint32_t const MAX_TEXTURES{ 10 };

		// !
		// common practice to rank from ‘least updated’ descriptor set(index 0) to ‘most frequent updated’
		// descriptor set(index N)!This way, we can avoid rebinding as much as possible!
		std::vector<VkDescriptorSet> m_DescriptorSets{};

		//TODO move to texturemanager
		size_t m_CurrTextureIdx{ SIZE_MAX };
	};
}

#endif