#ifndef MAUREN_VULKANLIGHTMANAGER_H
#define MAUREN_VULKANLIGHTMANAGER_H

#include "RendererPCH.h"
#include "VulkanImage.h"

#include "Assets/Light.h"
#include "Vulkan/VulkanBuffer.h"

namespace MauEng
{
	struct CLight;
}

namespace MauRen
{
	class VulkanSwapchainContext;
	class VulkanGraphicsPipelineContext;
	class VulkanDescriptorContext;

	class VulkanLightManager final : public MauCor::Singleton<VulkanLightManager>
	{
	public:
		void Initialize(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext);
		void Destroy();

		[[nodiscard]] uint32_t GetNumLights() const noexcept { return static_cast<uint32_t>(std::size(m_Lights)); }

		[[nodiscard]] uint32_t CreateLight();

		void Draw(VkCommandBuffer const& commandBuffer, VulkanGraphicsPipelineContext const& graphicsPipelineContext, VulkanDescriptorContext& descriptorContext, VulkanSwapchainContext& swapChainContext, uint32_t frame);

		void SetSceneAABBOverride(glm::vec3 const& min, glm::vec3 const& max);
		void PreQueue(glm::mat4 const& viewProj);
		void PreDraw(uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame);
		void QueueLight(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, MauEng::CLight const& light);
		void PostDraw();

		[[nodiscard]] VkSampler GetShadowMapSampler() const noexcept { return m_ShadowMapSampler; }

		VulkanLightManager(VulkanLightManager const&) = delete;
		VulkanLightManager(VulkanLightManager&&) = delete;
		VulkanLightManager& operator=(VulkanLightManager const&) = delete;
		VulkanLightManager& operator=(VulkanLightManager const&&) = delete;

	private:
		friend class MauCor::Singleton<VulkanLightManager>;
		VulkanLightManager() = default;
		virtual ~VulkanLightManager() override = default;

		uint32_t m_NextLightID{ 0 };
		uint32_t m_NextShadowMapID{ 1 };

		std::unordered_map<uint32_t, uint32_t> m_LightShadowMapIDMap;

		std::vector<Light> m_Lights; // All lights that are currently active
		std::vector<VulkanMappedBuffer> m_LightBuffers;

		// 1:1 copy of the shadow maps on GPU
		std::vector<VulkanImage> m_ShadowMaps;

		struct ShadowPassPushConstant final
		{
			uint32_t lightIndex;
		};

		glm::vec3 m_SceneAABBMin{};
		glm::vec3 m_SceneAABBMax{};

		bool m_HasAABBBOverride;

		VkSampler m_ShadowMapSampler{ VK_NULL_HANDLE };

		void CreateShadowMapSampler(VulkanDescriptorContext& descriptorContext);

		void CreateDefaultShadowMap(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, uint32_t width, uint32_t height);
		void CreateShadowMap(VulkanCommandPoolManager& cmdPoolManager, uint32_t width, uint32_t height);

		void InitLightBuffers();
	};
}
#endif