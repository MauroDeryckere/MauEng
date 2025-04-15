#include "VulkanMaterialManager.h"

#include "VulkanDescriptorContext.h"

#include "VulkanTextureManager.h"
#include "Material.h"

#include "VulkanDeviceContextManager.h"

namespace MauRen
{
	void VulkanMaterialManager::Initialize()
	{
		ME_PROFILE_FUNCTION()

		m_TextureManager = std::make_unique<VulkanTextureManager>();
		InitMaterialBuffers();
	}

	void VulkanMaterialManager::Destroy()
	{
		for (auto & m : m_MaterialDataBuffers)
		{
			m.buffer.Destroy();
		}
		
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

	uint32_t VulkanMaterialManager::LoadOrGetMaterial(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, Material const& material)
	{
		ME_PROFILE_FUNCTION()

		auto const it{ m_MaterialIDMap.find(material.name) };
		if (it != end(m_MaterialIDMap))
		{
			return it->second;
		}

		MaterialData vkMat{};
		vkMat.albedoTextureID = m_TextureManager->LoadOrGetTexture(cmdPoolManager, descriptorContext, material.diffuseTexture);
		m_Materials.emplace_back(vkMat);

		// Upload the material id if new
		{
			VkDeviceSize constexpr MAT_SIZE{ sizeof(MaterialData) };

			for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
			{
				// Write to CPU-visible buffer
				uint8_t* basePtr = static_cast<uint8_t*>(m_MaterialDataBuffers[i].mapped);
				std::memcpy(basePtr + (m_Materials.size() - 1) * MAT_SIZE, &vkMat, MAT_SIZE);

				// Describe buffer for descriptor
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = m_MaterialDataBuffers[i].buffer.buffer;
				bufferInfo.range = sizeof(MaterialData) * MAX_MATERIALS;

				//TODO simply do this once per frame, if buffer has changed
				descriptorContext.BindMaterialBuffer(bufferInfo, i);
			}
		}

		return static_cast<uint32_t>(m_Materials.size() - 1);
	}

	MaterialData const& VulkanMaterialManager::GetMaterial(uint32_t ID) const noexcept
	{
		return m_Materials[ID];
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
			vkMapMemory(deviceContext->GetLogicalDevice(), m_MaterialDataBuffers[i].buffer.bufferMemory, 0, BUFFER_SIZE, 0, &m_MaterialDataBuffers[i].mapped);
		}
	}
}
