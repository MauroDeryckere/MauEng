#ifndef MAUREN_VULKANLIGHTMANAGER_H
#define MAUREN_VULKANLIGHTMANAGER_H

#include "RendererPCH.h"
#include "VulkanImage.h"

#include "Assets/Light.h"

namespace MauEng
{
	struct CLight;
}

namespace MauRen
{
	class VulkanDescriptorContext;

	class VulkanLightManager final : public MauCor::Singleton<VulkanLightManager>
	{
	public:
		void Initialize();
		void Destroy();

		void AddLight(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, MauEng::CLight const& light);

		VulkanLightManager(VulkanLightManager const&) = delete;
		VulkanLightManager(VulkanLightManager&&) = delete;
		VulkanLightManager& operator=(VulkanLightManager const&) = delete;
		VulkanLightManager& operator=(VulkanLightManager const&&) = delete;

	private:
		friend class MauCor::Singleton<VulkanLightManager>;
		VulkanLightManager() = default;
		virtual ~VulkanLightManager() override = default;

		std::unordered_map<uint32_t, uint32_t> m_LightShadowMapIDMap;

		// TODO Light Buffer - GPU
		std::vector<Light> m_Lights; // All lights that are currently active

		// 1:1 copy of the shadow maps on GPU
		std::vector<VulkanImage> m_ShadowMaps;

		void CreateDefaultShadowMap();
		void CreateShadowMap();
	};
}
#endif