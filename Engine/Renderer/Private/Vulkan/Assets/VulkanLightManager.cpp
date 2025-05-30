#include "VulkanLightManager.h"

#include "../../MauEng/Public/Components/CLight.h"
#include "Vulkan/VulkanCommandPoolManager.h"
#include "Vulkan/VulkanDescriptorContext.h"

namespace MauRen
{
	void VulkanLightManager::Initialize(VulkanCommandPoolManager& cmdPoolManager)
	{
		CreateDefaultShadowMap(cmdPoolManager, 1024, 1024);
	}

	void VulkanLightManager::Destroy()
	{
		for (auto& s : m_ShadowMaps)
		{
			s.Destroy();
		}
	}

	void VulkanLightManager::AddLight(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, MauEng::CLight const& light)
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
				CreateShadowMap(cmdPoolManager, 1024, 1024);
				descriptorContext.BindShadowMap(shadowID, m_ShadowMaps[shadowID].imageViews[0], m_ShadowMaps[shadowID].layout);
			}
		}

		Light vulkanLight{};
		vulkanLight.type = static_cast<uint32_t>(light.type);
		vulkanLight.direction_position = light.direction_position;
		vulkanLight.color = light.lightColour;
		vulkanLight.intensity = light.intensity;
		vulkanLight.shadowMapIndex = shadowID;
		vulkanLight.castsShadows = light.castShadows ? 1 : 0;

		m_Lights.emplace_back(vulkanLight);
	}

	void VulkanLightManager::CreateDefaultShadowMap(VulkanCommandPoolManager& cmdPoolManager, uint32_t width, uint32_t height)
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
	}
}
