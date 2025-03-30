#include "VulkanDescriptorContext.h"

namespace MauRen
{
	// ! THIS IS NOT SAFE TO CALL DURING A FRAME, HAS TO BE HANDLED IF WE WANT THAT
	void VulkanDescriptorContext::AddTexture(VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = imageLayout;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = 1;
			descriptorWrite.dstArrayElement = static_cast<uint32_t>(m_CurrTextureIdx);
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
			vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

		}
		m_CurrTextureIdx = (m_CurrTextureIdx + 1) % MAX_TEXTURES;
	}

	void VulkanDescriptorContext::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		// This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example.
		// Our MVP transformation is in a single uniform buffer object, so we're using a descriptorCount of 1.
		uboLayoutBinding.descriptorCount = 1;

		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutBinding bindlessTextureBinding{};
		bindlessTextureBinding.binding = 1;
		bindlessTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindlessTextureBinding.descriptorCount = 2;
		bindlessTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindlessTextureBinding.pImmutableSamplers = nullptr;

		// Flags for the binding - only use valid flags for image samplers
		VkDescriptorBindingFlagsEXT bindingFlags[2] = {
			0 ,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT // Flags for bindless textures

		};

		std::array<VkDescriptorSetLayoutBinding, 2> const bindings{ uboLayoutBinding, bindlessTextureBinding };
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		bindingFlagsInfo.pBindingFlags = bindingFlags;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = &bindingFlagsInfo;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		if (vkCreateDescriptorSetLayout(deviceContext->GetLogicalDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
	}

	void VulkanDescriptorContext::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		// Assuming one set per frame and multiple textures
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		if (vkCreateDescriptorPool(deviceContext->GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {

			throw std::runtime_error("Failed to create descriptor pool!");
		}
	}

	void VulkanDescriptorContext::CreateDescriptorSets(std::vector<VulkanBuffer> const& bufferInfoBuffers, VkDeviceSize offset, VkDeviceSize range, VkImageLayout imageLayout, std::vector<VkImageView> const& imageViews, VkSampler sampler)
	{
		std::vector<VkDescriptorSetLayout> const layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();


		m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		if (vkAllocateDescriptorSets(deviceContext->GetLogicalDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkWriteDescriptorSet descriptorWrites[2] = {};

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = bufferInfoBuffers[i].buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = range;

			// Descriptor write for uniform buffer (binding 0)
			descriptorWrites[0] = {
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				m_DescriptorSets[i],
				0,  // Binding for the uniform buffer
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				nullptr,
				&bufferInfo,
				nullptr
				};

			uint32_t const textureCount{ std::min(static_cast<uint32_t>(imageViews.size()), MAX_TEXTURES) }; // Avoid exceeding MAX_TEXTURES
			m_CurrTextureIdx = textureCount;
			std::vector<VkDescriptorImageInfo> bindlessImageInfos(textureCount);
			for (size_t textureID = 0; textureID < textureCount; ++textureID)
			{
				bindlessImageInfos[textureID].imageLayout = imageLayout;
				bindlessImageInfos[textureID].imageView = imageViews[textureID];
				bindlessImageInfos[textureID].sampler = sampler;
			}

			VkWriteDescriptorSet descriptorWriteTextures{};
			descriptorWriteTextures.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWriteTextures.dstSet = m_DescriptorSets[i];
			descriptorWriteTextures.dstBinding = 1;  // Single binding for all textures
			descriptorWriteTextures.dstArrayElement = 0;
			descriptorWriteTextures.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWriteTextures.descriptorCount = textureCount;  // Number of textures in the array
			descriptorWriteTextures.pImageInfo = bindlessImageInfos.data();  // Array of image info

			descriptorWrites[1] = descriptorWriteTextures;

			vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), static_cast<uint32_t>(2), descriptorWrites, 0, nullptr);
		}
	}

	void VulkanDescriptorContext::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DescriptorPool, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DescriptorSetLayout, nullptr);
	}
}
