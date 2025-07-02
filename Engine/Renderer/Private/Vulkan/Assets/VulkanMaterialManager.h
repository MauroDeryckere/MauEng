#ifndef MAUREN_VULKANMATERIALMANAGER_H
#define MAUREN_VULKANMATERIALMANAGER_H

#include "RendererPCH.h"
#include "../VulkanBuffer.h"
#include "VulkanTextureManager.h"

#include "BindlessData.h"

namespace MauRen
{
	struct Material;
	class VulkanCommandPoolManager;
	class VulkanDescriptorContext;

	class VulkanMaterialManager final : public MauCor::Singleton<VulkanMaterialManager>
	{
	public:
		void Initialize();
		void InitializeTextureManager(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descContext);
		void Destroy();

		// Returns if it exists, and ID if it does
		[[nodiscard]] std::pair<bool, uint32_t> GetMaterial(std::string const& materialName) const noexcept;

		[[nodiscard]] uint32_t LoadOrGetMaterial(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, Material const& material);

		[[nodiscard]] VkSampler GetTextureSampler() const noexcept { return m_TextureManager->GetTextureSampler(); }
		[[nodiscard]] VulkanTextureManager* GetTextureManager() const noexcept { return m_TextureManager.get(); }
		[[nodiscard]] std::vector<MaterialData> const& GetMaterials() const noexcept { return m_Materials; }

		[[nodiscard]] std::unordered_map<std::string, LoadedMaterialInfo> const& GetMaterialIDMap() const noexcept { return m_MaterialIDMap; }
		[[nodiscard]] std::vector<MaterialData> const& GetMaterialData() const noexcept { return m_Materials; }

		VulkanMaterialManager(VulkanMaterialManager const&) = delete;
		VulkanMaterialManager(VulkanMaterialManager&&) = delete;
		VulkanMaterialManager& operator=(VulkanMaterialManager const&) = delete;
		VulkanMaterialManager& operator=(VulkanMaterialManager const&&) = delete;

	private:
		friend class MauCor::Singleton<VulkanMaterialManager>;
		VulkanMaterialManager() = default;
		virtual ~VulkanMaterialManager() override = default;
		
		std::unique_ptr<VulkanTextureManager> m_TextureManager;

		// Material name, ID in MaterialData[ ]
		std::unordered_map<std::string, LoadedMaterialInfo> m_MaterialIDMap;

		// All loaded materials - 1:1 copy of GPU buffer
		std::vector<MaterialData> m_Materials;
		std::vector<VulkanMappedBuffer> m_MaterialDataBuffers;

		uint32_t m_NextMaterialID{ 0 };

		void InitMaterialBuffers();

		void CreateDefaultMaterial(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descContext);
	};
}

#endif