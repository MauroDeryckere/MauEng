#include "VulkanDescriptorContext.h"

namespace MauRen
{
	// ! THIS IS NOT SAFE TO CALL DURING A FRAME, HAS TO BE HANDLED IF WE WANT THAT
	void VulkanDescriptorContext::AddTexture(uint32_t destLocation, VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = imageLayout;
		imageInfo.imageView = imageView;

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = TEXTURE_BINDING_SLOT;
			descriptorWrite.dstArrayElement = destLocation;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
			vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

		}
	}

	void VulkanDescriptorContext::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = UBO_BINDING_SLOT;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		// This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example.
		// Our MVP transformation is in a single uniform buffer object, so we're using a descriptorCount of 1.
		uboLayoutBinding.descriptorCount = 1;

		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		// Binding for global sampler
		VkDescriptorSetLayoutBinding samplerBinding{};
		samplerBinding.binding = SAMPLER_BINDING_SLOT;
		samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerBinding.descriptorCount = 1;
		samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding bindlessTextureBinding{};
		bindlessTextureBinding.binding = TEXTURE_BINDING_SLOT;
		bindlessTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		bindlessTextureBinding.descriptorCount = MAX_TEXTURES;
		bindlessTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 3> const bindings{ uboLayoutBinding, samplerBinding, bindlessTextureBinding };

		// Flags for the binding - only use valid flags for image samplers
		VkDescriptorBindingFlagsEXT bindingFlags[bindings.size()] = {
			0,
			0 ,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT // Flags for bindless textures

		};
		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
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
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		// Assuming one set per frame and multiple textures
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		// Assuming one set per frame and multiple textures
		poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT);

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
			VkWriteDescriptorSet descriptorWrites[3] = {};

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = bufferInfoBuffers[i].buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = range;

			// Descriptor write for uniform buffer (binding 0)
			descriptorWrites[UBO_BINDING_SLOT] = {
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				m_DescriptorSets[i],
				UBO_BINDING_SLOT,  // Binding for the uniform buffer
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				nullptr,
				&bufferInfo,
				nullptr
				};

			VkDescriptorImageInfo samplerInfo{};
			samplerInfo.sampler = sampler;
			samplerInfo.imageView = VK_NULL_HANDLE;
			samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			descriptorWrites[SAMPLER_BINDING_SLOT] = {
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				m_DescriptorSets[i],
				SAMPLER_BINDING_SLOT,
				0,
				SAMPLER_BINDING_SLOT,
				VK_DESCRIPTOR_TYPE_SAMPLER,
				&samplerInfo,
				nullptr,
				nullptr
			};

			uint32_t const textureCount{ std::min(static_cast<uint32_t>(imageViews.size()), MAX_TEXTURES) }; // Avoid exceeding MAX_TEXTURES
			std::vector<VkDescriptorImageInfo> bindlessImageInfos(textureCount);
			for (size_t textureID = 0; textureID < textureCount; ++textureID)
			{
				bindlessImageInfos[textureID].imageLayout = imageLayout;
				bindlessImageInfos[textureID].imageView = imageViews[textureID];
			}

			VkWriteDescriptorSet descriptorWriteTextures{};
			descriptorWriteTextures.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWriteTextures.dstSet = m_DescriptorSets[i];
			descriptorWriteTextures.dstBinding = TEXTURE_BINDING_SLOT;  // Single binding for all textures
			descriptorWriteTextures.dstArrayElement = 0;
			descriptorWriteTextures.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descriptorWriteTextures.descriptorCount = textureCount;  // Number of textures in the array
			descriptorWriteTextures.pImageInfo = bindlessImageInfos.data();  // Array of image info

			if (textureCount > 0)
			{
				descriptorWrites[TEXTURE_BINDING_SLOT] = descriptorWriteTextures;
			}

			vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), textureCount > 0 ? static_cast<uint32_t>(3) : 2, descriptorWrites, 0, nullptr);
		}
	}

	void VulkanDescriptorContext::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DescriptorPool, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_DescriptorSetLayout, nullptr);
	}
}
