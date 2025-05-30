#include "VulkanLightManager.h"

#include "VulkanMeshManager.h"
#include "../../MauEng/Public/Components/CLight.h"
#include "Vulkan/VulkanCommandPoolManager.h"
#include "Vulkan/VulkanDescriptorContext.h"
#include "Vulkan/VulkanGraphicsPipelineContext.h"

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

	void VulkanLightManager::Draw(VkCommandBuffer const& commandBuffer, VulkanGraphicsPipelineContext const& graphicsPipelineContext, VulkanDescriptorContext& descriptorContext, VulkanSwapchainContext& swapChainContext, uint32_t frame)
	{
		VkClearValue constexpr depthClear{ .depthStencil = { 1.0f, 0 } };

		for (uint32_t lightId{ 0 }; lightId < std::size(m_Lights); ++lightId)
		{
			auto const& light{ m_Lights[lightId] };
			if (light.castsShadows and light.type == static_cast<uint32_t>(MauEng::ELightType::DIRECTIONAL))
			{
				auto& depth = m_ShadowMaps[light.shadowMapIndex];

				if (VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL != depth.layout)
				{
					depth.TransitionImageLayout(
						commandBuffer,
						VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
						0,
						VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
					);
				}

				ShadowPassPushConstant pc{ .lightIndex = lightId };

				VkRenderingAttachmentInfo depthAttachment{};
				depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				depthAttachment.imageView = depth.imageViews[0];
				depthAttachment.imageLayout = depth.layout;
				depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				depthAttachment.clearValue = depthClear;

				VkRenderingInfo renderInfoDepthPrepass{};
				renderInfoDepthPrepass.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
				renderInfoDepthPrepass.renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE };
				renderInfoDepthPrepass.layerCount = 1;
				renderInfoDepthPrepass.colorAttachmentCount = 0;
				renderInfoDepthPrepass.pColorAttachments = nullptr;
				renderInfoDepthPrepass.pDepthAttachment = &depthAttachment;
				renderInfoDepthPrepass.pStencilAttachment = nullptr;

				vkCmdBeginRendering(commandBuffer, &renderInfoDepthPrepass);
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineContext.GetShadowPassPipeline());
					vkCmdPushConstants(
						commandBuffer,
						graphicsPipelineContext.GetShadowPassPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT,
						0,
						sizeof(ShadowPassPushConstant),
						&pc
					);

					VulkanMeshManager::GetInstance().Draw(commandBuffer, graphicsPipelineContext.GetShadowPassPipelineLayout(), 1, &descriptorContext.GetDescriptorSets()[frame], frame);
				vkCmdEndRendering(commandBuffer);

				depth.TransitionImageLayout(
					commandBuffer,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT
				);
			}
		}
	}

	void VulkanLightManager::PreDraw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame)
	{
		{
			ME_PROFILE_SCOPE("Light data update - buffer")
			memcpy(m_LightBuffers[frame].mapped, m_Lights.data(), m_Lights.size() * sizeof(Light));
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
				bufferInfo.range = m_Lights.size() * sizeof(Light);

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

		//TODO
		vulkanLight.lightViewProj = {};

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
