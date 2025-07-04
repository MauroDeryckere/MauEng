#include "VulkanMeshManager.h"

#include "MeshInstance.h"
#include "RendererIdentifiers.h"
#include "../VulkanDeviceContextManager.h"
#include "VulkanMaterialManager.h"

#include "Assets/ModelLoader.h"
#include "Vulkan/VulkanDescriptorContext.h"
#include "Vulkan/VulkanMemoryAllocator.h"

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
		for (auto & b : m_VertexBuffer)
		{
			b.UnMap();
			b.buffer.Destroy();
		}
		for (auto& b : m_IndexBuffer)
		{
			b.UnMap();
			b.buffer.Destroy();
		}

		for (auto& d : m_DrawCommandBuffers)
		{
			d.UnMap();
			d.buffer.Destroy();
		}

		for(auto& m : m_MeshInstanceDataBuffers)
		{
			m.UnMap();
			m.buffer.Destroy();
		}

		return true;
	}

	void VulkanMeshManager::UnloadMesh(uint32_t meshID) noexcept
	{
		ME_PROFILE_FUNCTION()

		auto const meshIt{ m_LoadedMeshes.find(meshID) };
		if (meshIt == m_LoadedMeshes.end())
		{
			ME_LOG_ERROR(LogRenderer, "Tried to unload non-existent mesh ID: {}", meshID);
			return;
		}

		uint32_t const internalIndex{ meshIt->second };
		std::string const path{ m_MeshID_path[meshID] };

		auto pathIt = m_LoadedMeshes_Path.find(path);
		ME_RENDERER_ASSERT(pathIt != m_LoadedMeshes_Path.end());

		pathIt->second.useCount--;

		// Still in use
		if (pathIt->second.useCount > 0)
		{
			return;
		}

		// Clear CPU-side model data if it somehow persisted
		m_CPUModelData.erase(meshID);

		MeshData& meshData{ m_MeshData[internalIndex] };
		// Clean up submeshes
		for (uint32_t i{ 0 }; i < meshData.subMeshCount; ++i)
		{
			VulkanMaterialManager::GetInstance().UnloadMaterial(m_SubMeshes[meshData.firstSubMesh + i].materialID);

			m_FreeIndices.emplace_back(m_SubMeshes[meshData.firstSubMesh + i].firstIndex, m_SubMeshes[meshData.firstSubMesh + i].indexCount);
			m_FreeVertices.emplace_back(m_SubMeshes[meshData.firstSubMesh + i].vertexOffset, m_SubMeshes[meshData.firstSubMesh + i].vertexCount);

			m_SubMeshes[meshData.firstSubMesh + i] = {}; // Zeroing out
		}
		FreeRange::MergeFreeRanges(m_FreeIndices);
		FreeRange::MergeFreeRanges(m_FreeVertices);
		
		m_MeshData[internalIndex] = {};

		m_LoadedMeshes.erase(meshIt);
		m_LoadedMeshes_Path.erase(pathIt);
		m_MeshID_path.erase(meshID);

		for (auto& frameUploads : m_PendingUploads)
		{
			frameUploads.erase(
				std::ranges::remove_if(frameUploads,
				                       [meshID](PendingMeshUpload const& pu) { return pu.meshID == meshID; }).begin(),
				frameUploads.end());
		}

		ME_LOG_INFO(LogRenderer, "Unloaded mesh ID: {} ({})", meshID, path);
	}

	uint32_t VulkanMeshManager::LoadMesh(char const* path, VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext) noexcept
	{
		ME_PROFILE_FUNCTION()

		std::string cleanPath{ path };
		std::string const prefix{ "Resources/Models/" };
		if (cleanPath.starts_with(prefix))
		{
			cleanPath.erase(0, prefix.size());
		}

		if (auto it{ m_LoadedMeshes_Path.find(cleanPath) }; it != m_LoadedMeshes_Path.end())
		{
			if (it->second.loadedMeshesID != INVALID_MESH_ID)
			{
				it->second.useCount++;
				return m_MeshData[it->second.loadedMeshesID].meshID;
			}
		}

		LoadedModel const loadedModel{ ModelLoader::LoadModel(path, cmdPoolManager, descriptorContext) };
		//ME_RENDERER_ASSERT(m_CurrentVertexOffset + loadedModel.vertices.size() <= MAX_VERTICES);
		//ME_RENDERER_ASSERT(m_CurrentIndexOffset + loadedModel.indices.size() <= MAX_INDICES);

		uint32_t const vertexCount{ static_cast<uint32_t>(loadedModel.vertices.size()) };
		uint32_t const indexCount{ static_cast<uint32_t>(loadedModel.indices.size()) };

		auto [usedVertexRange, vertexOffset] { FreeRange::TryUseFreeRange(m_FreeVertices, vertexCount) };
		auto [usedIndexRange, indexOffset] { FreeRange::TryUseFreeRange(m_FreeIndices, indexCount) };

		if (!usedVertexRange)
		{
			vertexOffset = m_CurrentVertexOffset;
			m_CurrentVertexOffset += vertexCount;
		}
		else
		{
			ME_LOG_INFO(LogRenderer, "Reusing free range in vertex buffer for mesh: {}", path);
		}
		if (!usedIndexRange)
		{
			indexOffset = m_CurrentIndexOffset;
			m_CurrentIndexOffset += indexCount;
		}
		else
		{
			ME_LOG_INFO(LogRenderer, "Reusing free range in index buffer for mesh: {}", path);
		}

		MeshData meshData;
		meshData.meshID = m_NextID;
		meshData.firstSubMesh = m_SubMeshes.size();
		meshData.subMeshCount = loadedModel.subMeshes.size();

		// Offset each submesh
		for (auto& sub : loadedModel.subMeshes)
		{
			SubMeshData entry{ sub };
			entry.vertexOffset += vertexOffset;
			entry.firstIndex += indexOffset;

			m_SubMeshes.emplace_back(entry);
		}

		m_CPUModelData[m_NextID] =
		{
			.vertices = loadedModel.vertices,
			.indices = loadedModel.indices,

			.vertexOffset = vertexOffset,
			.indexOffset = indexOffset,

			.uses = 3
		};

		for (auto& pu : m_PendingUploads)
		{
			pu.emplace_back(m_NextID);
		}

		m_CurrentVertexOffset += static_cast<uint32_t>(loadedModel.vertices.size());
		m_CurrentIndexOffset += static_cast<uint32_t>(loadedModel.indices.size());

		m_LoadedMeshes[m_NextID] = static_cast<uint32_t>(m_MeshData.size());
		m_LoadedMeshes_Path[cleanPath] = { static_cast<uint32_t>(m_MeshData.size()), 1 };
		m_MeshID_path[m_NextID] = cleanPath;

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

	void VulkanMeshManager::PreDraw(VulkanDescriptorContext& descriptorContext, uint32_t frame)
	{
		{
			ME_PROFILE_SCOPE("Mesh GPU uploads")

				// Update the vertex & index buffer
				for (auto& upload : m_PendingUploads[frame])
				{
					auto& data{ m_CPUModelData.at(upload.meshID) };
					{
						uint8_t* basePtr{ static_cast<uint8_t*>(m_VertexBuffer[frame].mapped) };

						std::memcpy(basePtr + data.vertexOffset * sizeof(Vertex),
							data.vertices.data(),
							data.vertices.size() * sizeof(Vertex));
					}
					{
						uint8_t* basePtr{ static_cast<uint8_t*>(m_IndexBuffer[frame].mapped) };

						std::memcpy(basePtr + data.indexOffset * sizeof(uint32_t),
							data.indices.data(),
							data.indices.size() * sizeof(uint32_t));
					}

					data.uses--;

					if (data.uses == 0)
					{
						m_CPUModelData.erase(upload.meshID);
					}
				}
			m_PendingUploads[frame].clear();
		}

		//TODO only does this when contents change
		{
			ME_PROFILE_SCOPE("Mesh instance data update - buffer")

			memcpy(m_MeshInstanceDataBuffers[frame].mapped, m_MeshInstanceData.data(), m_MeshInstanceData.size() * sizeof(MeshInstanceData));
		}

		{
			ME_PROFILE_SCOPE("Draw commands data update - buffer")

			memcpy(m_DrawCommandBuffers[frame].mapped, m_DrawCommands.data(), m_DrawCommands.size() * sizeof(DrawCommand));
		}

		//TODO only does this when contents change
		{
			if (not m_MeshInstanceData.empty())
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = m_MeshInstanceDataBuffers[frame].buffer.buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = m_MeshInstanceData.size() * sizeof(MeshInstanceData);

				descriptorContext.BindMeshInstanceDataBuffer(bufferInfo, frame);
			}
		}
	}

	void VulkanMeshManager::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame)
	{
		ME_PROFILE_FUNCTION()

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, setCount, pDescriptorSets, 0, nullptr);
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer[frame].buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		VkDeviceSize offset{ 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer[frame].buffer.buffer, &offset);
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
			vmaMapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), m_MeshInstanceDataBuffers[i].buffer.alloc, &m_MeshInstanceDataBuffers[i].mapped);
		}
	}

	void VulkanMeshManager::InitializeDrawCommandBuffers() noexcept
	{
		VkDeviceSize constexpr BUFFER_SIZE{ sizeof(DrawCommand) * MAX_DRAW_COMMANDS };

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_DrawCommandBuffers.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT ,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vmaMapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), m_DrawCommandBuffers[i].buffer.alloc, &m_DrawCommandBuffers[i].mapped);
		}
	}

	void VulkanMeshManager::CreateVertexAndIndexBuffers() noexcept
	{
		for (size_t i { 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkDeviceSize constexpr BUFFER_SIZE{ sizeof(Vertex) * MAX_VERTICES };

			m_VertexBuffer.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vmaMapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), m_VertexBuffer.back().buffer.alloc, &m_VertexBuffer.back().mapped);
		}

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkDeviceSize constexpr BUFFER_SIZE{ sizeof(uint32_t) * MAX_INDICES };

			m_IndexBuffer.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vmaMapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), m_IndexBuffer.back().buffer.alloc, &m_IndexBuffer.back().mapped);
		}
	}
}
