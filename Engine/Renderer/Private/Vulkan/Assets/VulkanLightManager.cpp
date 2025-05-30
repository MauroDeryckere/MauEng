#include "VulkanLightManager.h"

#include "../../MauEng/Public/Components/CLight.h"
#include "Vulkan/VulkanCommandPoolManager.h"
#include "Vulkan/VulkanDescriptorContext.h"

namespace MauRen
{
	void VulkanLightManager::Initialize(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext)
	{
		ME_PROFILE_FUNCTION()

		CreateDefaultShadowMap(cmdPoolManager, descriptorContext, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

		InitLightBuffers();
	}

	void VulkanLightManager::Destroy()
	{
		for (auto& s : m_ShadowMaps)
		{
			s.Destroy();
		}

		for (auto& l : m_LightBuffers)
		{
			l.buffer.Destroy();
		}
	}

	uint32_t VulkanLightManager::CreateLight()
	{
		return m_NextLightID++;
	}

	void VulkanLightManager::PreDraw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame)
	{
		{
			ME_PROFILE_SCOPE("Light data update - buffer")
			memcpy(m_LightBuffers[frame].mapped, m_Lights.data(), m_Lights.size() * sizeof(MeshInstanceData));
		}

		{
			ME_PROFILE_SCOPE("Light instance data update - descriptor sets")

			// Will likely never be empty but if it is, skip to prevent errors
			if (not m_Lights.empty())
			{
				auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = m_LightBuffers[frame].buffer.buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = m_Lights.size() * sizeof(MeshInstanceData);

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = *pDescriptorSets;
				descriptorWrite.dstBinding = 12; // Binding index -TODO use a get Binding on the context
				descriptorWrite.dstArrayElement = 0; // Array element offset (if applicable)
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}

		//TODO transition all the imgs to depth attachment
	}

	void VulkanLightManager::QueueLight(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, MauEng::CLight const& light)
	{
		if (not light.isEnabled)
		{
			return;
		}

		uint32_t shadowID{ INVALID_SHADOW_MAP_ID };

		if (light.castShadows)
		{
			auto const it{ m_LightShadowMapIDMap.find(light.lightID) };
			if (it != end(m_LightShadowMapIDMap))
			{
				shadowID = it->second;
			}
			else
			{
				shadowID = m_NextShadowMapID;
				CreateShadowMap(cmdPoolManager, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
				descriptorContext.BindShadowMap(shadowID, m_ShadowMaps[shadowID].imageViews[0], m_ShadowMaps[shadowID].layout);

				m_LightShadowMapIDMap.emplace(light.lightID, shadowID);
				m_NextShadowMapID++;
			}
		}

		Light vulkanLight;
		vulkanLight.type = static_cast<uint32_t>(light.type);
		vulkanLight.direction_position = light.direction_position;
		vulkanLight.color = light.lightColour;
		vulkanLight.intensity = light.intensity;
		vulkanLight.shadowMapIndex = shadowID;
		vulkanLight.castsShadows = light.castShadows ? 1 : 0;

		m_Lights.emplace_back(vulkanLight);
	}

	void VulkanLightManager::PostDraw()
	{
		m_Lights.clear();
	}

	void VulkanLightManager::CreateDefaultShadowMap(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, uint32_t width, uint32_t height)
	{
		VulkanImage shadowImage
		{
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			width,
			height,
			1
		};

		shadowImage.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

		// Transition to transfer dst for clearing
		shadowImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkClearDepthStencilValue clearValue{};
		clearValue.depth = 1.0f;
		clearValue.stencil = 0;

		VkImageSubresourceRange range{};
		range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		auto cmd{ cmdPoolManager.BeginSingleTimeCommands() };
		vkCmdClearDepthStencilImage(
			cmd,
			shadowImage.image,
			shadowImage.layout,
			&clearValue,
			1,
			&range);

		cmdPoolManager.EndSingleTimeCommands(cmd);

		shadowImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_ShadowMaps.emplace_back(shadowImage);

		descriptorContext.BindShadowMap(0, m_ShadowMaps[0].imageViews[0], m_ShadowMaps[0].layout);
	}

	void VulkanLightManager::CreateShadowMap(VulkanCommandPoolManager& cmdPoolManager, uint32_t width, uint32_t height)
	{
		VulkanImage shadowImage
		{
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			width,
			height,
			1
		};

		shadowImage.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

		m_ShadowMaps.emplace_back(shadowImage);

		m_ShadowMaps.back().TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanLightManager::InitLightBuffers()
	{
		auto deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize constexpr BUFFER_SIZE{ sizeof(Light) * MAX_LIGHTS };

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_LightBuffers.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vkMapMemory(deviceContext->GetLogicalDevice(), m_LightBuffers[i].buffer.bufferMemory, 0, BUFFER_SIZE, 0, &m_LightBuffers[i].mapped);
		}
	}
}
