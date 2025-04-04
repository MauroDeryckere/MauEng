#ifndef MAUREN_VULKANMATERIALMANAGER_H
#define MAUREN_VULKANMATERIALMANAGER_H

#include "RendererPCH.h"
#include "VulkanMaterial.h"
#include "VulkanTextureManager.h"

namespace MauRen
{
	struct Material;
	class VulkanCommandPoolManager;
	class VulkanDescriptorContext;

	class VulkanMaterialManager final : public MauCor::Singleton<VulkanMaterialManager>
	{
	public:
		void Initialize();
		void Destroy();

		[[nodiscard]] bool Exists(uint32_t ID) const noexcept;

		[[nodiscard]] uint32_t LoadMaterial(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, Material const& material);

		[[nodiscard]] VulkanMaterial const& GetMaterial(uint32_t ID) const noexcept;

		[[nodiscard]] VkSampler GetTextureSampler() const noexcept { return m_TextureManager->GetTextureSampler(); }
		[[nodiscard]] VulkanTextureManager* GetTextureManager() const noexcept { return m_TextureManager.get(); }
		[[nodiscard]] std::vector<VulkanMaterial> const& GetMaterials() const noexcept { return m_Materials; }

		VulkanMaterialManager(VulkanMaterialManager const&) = delete;
		VulkanMaterialManager(VulkanMaterialManager&&) = delete;
		VulkanMaterialManager& operator=(VulkanMaterialManager const&) = delete;
		VulkanMaterialManager& operator=(VulkanMaterialManager const&&) = delete;

	private:
		friend class MauCor::Singleton<VulkanMaterialManager>;
		VulkanMaterialManager() = default;
		virtual ~VulkanMaterialManager() override = default;

		std::unique_ptr<VulkanTextureManager> m_TextureManager;

		// Material name, ID
		std::unordered_map<std::string, uint32_t> m_MaterialIDMap;

		// All loaded materials
		std::vector<VulkanMaterial> m_Materials;

	};
}

#endif