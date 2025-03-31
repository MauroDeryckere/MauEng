#include "VulkanMaterialManager.h"

#include "VulkanDescriptorContext.h"

#include "VulkanTextureManager.h"
#include "VulkanMaterial.h"
#include "Material.h"

namespace MauRen
{
	void VulkanMaterialManager::Initialize()
	{
		m_TextureManager = std::make_unique<VulkanTextureManager>();
	}

	void VulkanMaterialManager::Destroy()
	{
		m_TextureManager = nullptr;
	}

	bool VulkanMaterialManager::Exists(uint32_t ID) const noexcept
	{
		if (ID == UINT32_MAX)
		{
			return false;
		}

		if (ID < m_Materials.size())
		{
			return true;
		}

		return false;
	}

	uint32_t VulkanMaterialManager::LoadMaterial(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, Material const& material)
	{
		auto it{ m_MaterialIDMap.find(material.name) };
		if (it != end(m_MaterialIDMap))
		{
			return it->second;
		}

		VulkanMaterial vkMat{};
		vkMat.albedoTexture = m_TextureManager->LoadOrGetTexture(cmdPoolManager, descriptorContext, material.diffuseTexture);
		m_Materials.emplace_back(vkMat);
		
		return m_Materials.size() - 1;
	}

	VulkanMaterial const& VulkanMaterialManager::GetMaterial(uint32_t ID) const noexcept
	{
		return m_Materials[ID];
	}
}