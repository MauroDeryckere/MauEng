#include "VulkanLightManager.h"

#include "../../MauEng/Public/Components/CLight.h"
#include "Vulkan/VulkanDescriptorContext.h"

namespace MauRen
{
	void VulkanLightManager::Initialize()
	{
		CreateDefaultShadowMap();
	}

	void VulkanLightManager::Destroy()
	{

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
				CreateShadowMap();
				// TODO bind
				//descriptorContext.BindShadowMap(shadowID)
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

	void VulkanLightManager::CreateDefaultShadowMap()
	{

	}

	void VulkanLightManager::CreateShadowMap()
	{

	}
}
