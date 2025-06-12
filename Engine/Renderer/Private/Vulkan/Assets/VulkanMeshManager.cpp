#include "VulkanMeshManager.h"

#include "MeshInstance.h"
#include "RendererIdentifiers.h"
#include "../VulkanDeviceContextManager.h"
#include "VulkanMaterialManager.h"

#include "Assets/ModelLoader.h"

namespace MauRen
{
	bool VulkanMeshManager::Initialize(VulkanCommandPoolManager const* CmdPoolManager)
	{
		m_CmdPoolManager = CmdPoolManager;

		m_MeshData.reserve(MAX_MESHES);
		m_SubMeshes.reserve(MAX_MESHES);

		m_MeshInstanceDataBuffers.reserve(MAX_MESH_INSTANCES);
		InitializeMeshInstanceDataBuffers();

		m_DrawCommands.reserve(MAX_DRAW_COMMANDS);
		InitializeDrawCommandBuffers();

		CreateVertexAndIndexBuffers();

		m_BatchedDrawCommands.reserve(MAX_MESHES + 1);
		m_BatchedDrawCommands.assign(MAX_MESHES + 1, INVALID_DRAW_COMMAND);

		return true;
	}

	bool VulkanMeshManager::Destroy()
	{
		m_VertexBuffer.buffer.Destroy();
		m_IndexBuffer.buffer.Destroy();

		for (auto& d : m_DrawCommandBuffers)
		{
			d.buffer.Destroy();
		}

		for(auto& m : m_MeshInstanceDataBuffers)
		{
			m.buffer.Destroy();
		}

		return true;
	}

	//NOT THREAD SAFE CURRENTLY, but okay to call at start program
	uint32_t VulkanMeshManager::LoadMesh(char const* path, VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext) noexcept
	{
		ME_PROFILE_FUNCTION()

		if (auto it{ m_LoadedMeshes_Path.find(path) }; it != m_LoadedMeshes_Path.end())
		{
			const auto& data{ m_MeshData[it->second] };
			return data.meshID;
		}

		LoadedModel const loadedModel{ ModelLoader::LoadModel({ path }, cmdPoolManager, descriptorContext) };
		ME_RENDERER_ASSERT(m_CurrentVertexOffset + loadedModel.vertices.size() <= MAX_VERTICES);
		ME_RENDERER_ASSERT(m_CurrentIndexOffset + loadedModel.indices.size() <= MAX_INDICES);

		MeshData meshData;
		meshData.meshID = m_NextID;
		meshData.firstSubMesh = m_SubMeshes.size();
		meshData.subMeshCount = loadedModel.subMeshes.size();

		// Offset each submesh
		for (auto& sub : loadedModel.subMeshes)
		{
			SubMeshData entry{ sub };
			entry.vertexOffset += m_CurrentVertexOffset;
			entry.firstIndex += m_CurrentIndexOffset;

			m_SubMeshes.emplace_back(entry);
		}

		// may want to store a copy of the buffers on the CPU  side to support compacting and be more "optimal" as its less copies.
		{
			uint8_t* basePtr{ static_cast<uint8_t*>(m_VertexBuffer.mapped) };

			std::memcpy(basePtr + m_CurrentVertexOffset * sizeof(Vertex), 
						loadedModel.vertices.data(), 
						loadedModel.vertices.size() * sizeof(Vertex));
		}
		{
			uint8_t* basePtr{ static_cast<uint8_t*>(m_IndexBuffer.mapped) };

			std::memcpy(basePtr + m_CurrentIndexOffset * sizeof(uint32_t), 
						loadedModel.indices.data(), 
						loadedModel.indices.size() * sizeof(uint32_t));
		}

		m_CurrentVertexOffset += static_cast<uint32_t>(loadedModel.vertices.size());
		m_CurrentIndexOffset += static_cast<uint32_t>(loadedModel.indices.size());

		m_LoadedMeshes[m_NextID] = static_cast<uint32_t>(m_MeshData.size());
		m_LoadedMeshes_Path[path] = static_cast<uint32_t>(m_MeshData.size());

		m_MeshData.emplace_back(std::move(meshData));

		return m_NextID++;
	}

	MeshData const& VulkanMeshManager::GetMeshData(uint32_t meshID) const
	{
		auto const it{ m_LoadedMeshes.find(meshID) };

		ME_RENDERER_ASSERT(it != end(m_LoadedMeshes), "Mesh not found in VulkanMeshManager");

		if (it != m_LoadedMeshes.end())
		{
			return m_MeshData[it->second];
		}

		throw std::runtime_error("Mesh not found! ");
	}

	void VulkanMeshManager::PreDraw(uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame)
	{
		{
			ME_PROFILE_SCOPE("Mesh instance data update - buffer")

			memcpy(m_MeshInstanceDataBuffers[frame].mapped, m_MeshInstanceData.data(), m_MeshInstanceData.size() * sizeof(MeshInstanceData));
		}

		{
			ME_PROFILE_SCOPE("Draw commands data update - buffer")

			memcpy(m_DrawCommandBuffers[frame].mapped, m_DrawCommands.data(), m_DrawCommands.size() * sizeof(DrawCommand));
		}

		{
			if (not m_MeshInstanceData.empty())
			{

				ME_PROFILE_SCOPE("Mesh instance data update - descriptor sets")

				auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = m_MeshInstanceDataBuffers[frame].buffer.buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = m_MeshInstanceData.size() * sizeof(MeshInstanceData);

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = *pDescriptorSets;
				descriptorWrite.dstBinding = 5; // Binding index -TODO use a get Binding on the context
				descriptorWrite.dstArrayElement = 0; // Array element offset (if applicable)
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
	}

	void VulkanMeshManager::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame)
	{
		ME_PROFILE_FUNCTION()

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, setCount, pDescriptorSets, 0, nullptr);
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		VkDeviceSize offset{ 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer.buffer.buffer, &offset);
		vkCmdDrawIndexedIndirect(
			commandBuffer,
			m_DrawCommandBuffers[frame].buffer.buffer,               // Indirect buffer that holds the draw command(s)
			0,														 // Offset into the indirect buffer
			static_cast<uint32_t>(m_DrawCommands.size()),			 // Number of draw commands to execute
			sizeof(DrawCommand)
		);
	}

	void VulkanMeshManager::PostDraw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame)
	{
		{
			ME_PROFILE_SCOPE("Clearing the data")

			// not optimal, good enough for now - just rebuild all draw commands every frame and queue them
			m_DrawCommands.resize(0);
			m_MeshInstanceData.resize(0);

			m_BatchedDrawCommands.assign(MAX_MESHES + 1, INVALID_MESH_ID);
		}
	}

	void VulkanMeshManager::InitializeMeshInstanceDataBuffers() noexcept
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize constexpr BUFFER_SIZE{ sizeof(MeshInstanceData) * MAX_MESH_INSTANCES };

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_MeshInstanceDataBuffers.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vkMapMemory(deviceContext->GetLogicalDevice(), m_MeshInstanceDataBuffers[i].buffer.bufferMemory, 0, BUFFER_SIZE, 0, &m_MeshInstanceDataBuffers[i].mapped);
		}
	}

	void VulkanMeshManager::InitializeDrawCommandBuffers() noexcept
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize constexpr BUFFER_SIZE{ sizeof(DrawCommand) * MAX_DRAW_COMMANDS };

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_DrawCommandBuffers.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT ,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vkMapMemory(deviceContext->GetLogicalDevice(), m_DrawCommandBuffers[i].buffer.bufferMemory, 0, BUFFER_SIZE, 0, &m_DrawCommandBuffers[i].mapped);
		}
	}

	void VulkanMeshManager::CreateVertexAndIndexBuffers() noexcept
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		{
			VkDeviceSize constexpr BUFFER_SIZE{ sizeof(Vertex) * MAX_VERTICES };

			m_VertexBuffer = (VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vkMapMemory(deviceContext->GetLogicalDevice(), m_VertexBuffer.buffer.bufferMemory, 0, BUFFER_SIZE, 0, &m_VertexBuffer.mapped);
		}

		{
			VkDeviceSize constexpr BUFFER_SIZE{ sizeof(uint32_t) * MAX_INDICES };

			m_IndexBuffer = (VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vkMapMemory(deviceContext->GetLogicalDevice(), m_IndexBuffer.buffer.bufferMemory, 0, BUFFER_SIZE, 0, &m_IndexBuffer.mapped);
		}
	}
}
