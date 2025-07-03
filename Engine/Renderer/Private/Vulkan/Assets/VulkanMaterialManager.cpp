#include "VulkanMaterialManager.h"

#include "../VulkanDescriptorContext.h"

#include "VulkanTextureManager.h"
#include "Assets/Material.h"

#include "../VulkanDeviceContextManager.h"
#include "Vulkan/VulkanMemoryAllocator.h"

namespace MauRen
{
	void VulkanMaterialManager::Initialize()
	{
		ME_PROFILE_FUNCTION()

		m_TextureManager = std::make_unique<VulkanTextureManager>();
		InitMaterialBuffers();
	}


	void VulkanMaterialManager::InitializeTextureManager(VulkanCommandPoolManager& cmdPoolManager,
		VulkanDescriptorContext& descContext)
	{
		ME_PROFILE_FUNCTION()

		m_TextureManager->InitializeTextures(cmdPoolManager, descContext);

		CreateDefaultMaterial(cmdPoolManager, descContext);
	}

	void VulkanMaterialManager::Destroy()
	{
		for (auto & m : m_MaterialDataBuffers)
		{
			m.UnMap();
			m.buffer.Destroy();
		}
		
		m_TextureManager = nullptr;
	}

	void VulkanMaterialManager::PreDraw(uint32_t currentFrame, VulkanDescriptorContext& descriptorContext)
	{
		m_TextureManager->PreDraw(currentFrame);

		if (m_DirtyMaterialIndices[currentFrame].empty())
		{
			return;
		}

		VkDeviceSize constexpr MAT_SIZE{ sizeof(MaterialData) };
		uint8_t* basePtr{ static_cast<uint8_t*>(m_MaterialDataBuffers[currentFrame].mapped) };

		for (uint32_t const index : m_DirtyMaterialIndices[currentFrame])
		{
			std::memcpy(basePtr + index * MAT_SIZE, &m_Materials[index], MAT_SIZE);
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_MaterialDataBuffers[currentFrame].buffer.buffer;
		bufferInfo.range = sizeof(MaterialData) * m_Materials.size();

		descriptorContext.BindMaterialBuffer(bufferInfo, currentFrame);

		m_DirtyMaterialIndices[currentFrame].clear();
	}

	std::pair<bool, uint32_t> VulkanMaterialManager::GetMaterial(std::string const& materialName) const noexcept
	{
		auto const it{ m_MaterialIDMap.find(materialName) };
		if (it == end(m_MaterialIDMap))
		{
			return { false, INVALID_MATERIAL_ID };
		}

		return { true, it->second.materialID };
	}

	void VulkanMaterialManager::UnloadMaterial(uint32_t materialID) noexcept
	{
		ME_PROFILE_FUNCTION()

		//TODO
		//m_TextureManager->UnloadTexture();
		//m_TextureManager->UnloadTexture();
		//m_TextureManager->UnloadTexture();
	}

	uint32_t VulkanMaterialManager::LoadOrGetMaterial(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, Material const& material)
	{
		ME_PROFILE_FUNCTION()

		auto const it{ m_MaterialIDMap.find(material.name) };
		if (it != end(m_MaterialIDMap))
		{
			m_MaterialIDMap[material.name].useCount++;
			return it->second.materialID;
		}

		MaterialData vkMat{};

		if (material.embDiffuse)
		{
			vkMat.albedoTextureID = m_TextureManager->LoadOrGetTexture(cmdPoolManager, descriptorContext, material.embDiffuse.hash, material.embDiffuse, false);
		}
		else
		{
			vkMat.albedoTextureID = m_TextureManager->LoadOrGetTexture(cmdPoolManager, descriptorContext, material.diffuseTexture, false);
		}

		if (material.embNormal)
		{
			vkMat.normalTextureID = m_TextureManager->LoadOrGetTexture(cmdPoolManager, descriptorContext, material.embNormal.hash, material.embNormal, true);
		}
		else
		{
			vkMat.normalTextureID = m_TextureManager->LoadOrGetTexture(cmdPoolManager, descriptorContext, material.normalMap, true);
		}

		if (material.embMetalnessRoughness)
		{
			vkMat.metallicTextureID = m_TextureManager->LoadOrGetTexture(cmdPoolManager, descriptorContext, material.embMetalnessRoughness.hash, material.embMetalnessRoughness, true);
		}
		else
		{
			vkMat.metallicTextureID = m_TextureManager->LoadOrGetTexture(cmdPoolManager, descriptorContext, material.metalnessRoughnessTexture, true);
		}

		vkMat.materialID = m_NextMaterialID;

		m_Materials.emplace_back(vkMat);
		m_MaterialIDMap[material.name] = { static_cast<uint32_t>(m_Materials.size() - 1), 1};
		m_NextMaterialID++;

		// Upload the material id if new
		for (auto& s : m_DirtyMaterialIndices)
		{
			s.emplace(m_Materials.size() - 1);
		}

		return static_cast<uint32_t>(m_Materials.size() - 1);
	}

	void VulkanMaterialManager::InitMaterialBuffers()
	{
		auto deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize constexpr BUFFER_SIZE{ sizeof(MaterialData) * MAX_MATERIALS};

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_MaterialDataBuffers.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vmaMapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), m_MaterialDataBuffers[i].buffer.alloc, &m_MaterialDataBuffers[i].mapped);
		}
	}

	void VulkanMaterialManager::CreateDefaultMaterial(VulkanCommandPoolManager& cmdPoolManager,
		VulkanDescriptorContext& descContext)
	{
		Material const defaultMat{};
		auto const id{ LoadOrGetMaterial(cmdPoolManager, descContext, defaultMat) };

		ME_ASSERT(id == DEFAULT_MATERIAL_ID);
	}
}
