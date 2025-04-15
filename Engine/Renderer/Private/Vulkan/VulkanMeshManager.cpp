#include "VulkanMeshManager.h"

#include "MeshInstance.h"
#include "VulkanDeviceContextManager.h"
#include "VulkanMaterialManager.h"

namespace MauRen
{
	bool VulkanMeshManager::Initialize(VulkanCommandPoolManager const* CmdPoolManager)
	{
		m_CmdPoolManager = CmdPoolManager;

		m_MeshData.reserve(MAX_MESHES);
		InitializeMeshDataBuffers();

		m_MeshInstanceDataBuffers.reserve(MAX_MESH_INSTANCES);
		InitializeMeshInstanceDataBuffers();

		m_DrawCommands.reserve(MAX_DRAW_COMMANDS);
		InitializeDrawCommandBuffers();

		CreateVertexAndIndexBuffers();

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

		for (auto& m : m_MeshDataBuffers)
		{
			m.buffer.Destroy();
		}

		return true;
	}

	//NOT THREAD SAFE CURRENTLY, but okay to call at start program
	void VulkanMeshManager::LoadMesh(Mesh& mesh)
	{
		ME_PROFILE_FUNCTION()
		if (mesh.GetMeshID() == UINT32_MAX)
		{
			auto const it{ m_LoadedMeshes.find(mesh.GetMeshID()) };
			if (it == end(m_LoadedMeshes))
			{
				mesh.SetMeshID(m_NextID);

				const auto& indices = mesh.GetIndices();
				const auto& vertices = mesh.GetVertices();

				ME_RENDERER_ASSERT(m_CurrentVertexOffset + vertices.size() <= sizeof(Vertex) * MAX_VERTICES);
				ME_RENDERER_ASSERT(m_CurrentIndexOffset + indices.size() <= sizeof(uint32_t) * MAX_INDICES);

				MeshData data{};
				data.vertexOffset = static_cast<int32_t>(m_CurrentVertexOffset);
				data.firstIndex = m_CurrentIndexOffset;
				data.indexCount = static_cast<uint32_t>(mesh.GetIndices().size());
				data.flags = 0;

				m_LoadedMeshes[m_NextID] = static_cast<uint32_t>(m_MeshData.size());
				m_MeshData.emplace_back(data);

				// may want to store a copy of the buffers on the CPU  side to support compacting and be more "optimal" as its less copies.
				{
					uint8_t* basePtr = static_cast<uint8_t*>(m_VertexBuffer.mapped);
					std::memcpy(basePtr + m_CurrentVertexOffset * sizeof(Vertex), vertices.data(), vertices.size() * sizeof(Vertex));
				}

				{
					uint8_t* basePtr = static_cast<uint8_t*>(m_IndexBuffer.mapped);
					std::memcpy(basePtr + m_CurrentIndexOffset * sizeof(uint32_t), indices.data(), indices.size() * sizeof(uint32_t));
				}

				m_CurrentVertexOffset += static_cast<uint32_t>(mesh.GetVertices().size());
				m_CurrentIndexOffset += static_cast<uint32_t>(mesh.GetIndices().size());


				++m_NextID;
			}
		}
	}

	MeshData const& VulkanMeshManager::GetMesh(uint32_t meshID) const
	{
		auto const it{ m_LoadedMeshes.find(meshID) };

		ME_RENDERER_ASSERT(it != end(m_LoadedMeshes), "Mesh not found in VulkanMeshManager");

		if (it != m_LoadedMeshes.end())
		{
			return m_MeshData[it->second];
		}

		throw std::runtime_error("Mesh not found! ");
	}

	void VulkanMeshManager::QueueDraw(MeshInstance const* instance)
	{
		uint32_t const meshID{ instance->GetMeshID() };
		uint32_t const instanceOffset{ static_cast<uint32_t>(m_MeshInstanceData.size()) };

		MeshInstanceData data{};
		data.modelMatrix = instance->GetModelMatrix();
		data.materialIndex = instance->GetMaterialID();
		m_MeshInstanceData.emplace_back(data);

		auto const it{ m_BatchedDrawCommands.find(meshID) };
		if (it != end(m_BatchedDrawCommands))
		{
			// Already added this mesh this frame; just increment instance count
			uint32_t const cmdIndex{ it->second };
			m_DrawCommands[cmdIndex].instanceCount++;
		}

		else
		{
			// First time seeing this mesh this frame; create a new draw command
			const MeshData& mesh{ m_MeshData[m_LoadedMeshes.at(meshID)] };

			DrawCommand cmd{};
			cmd.indexCount = mesh.indexCount;
			cmd.firstIndex = mesh.firstIndex;
			cmd.vertexOffset = mesh.vertexOffset;
			cmd.firstInstance = instanceOffset;
			cmd.instanceCount = 1;

			m_BatchedDrawCommands[meshID] = static_cast<uint32_t>(m_DrawCommands.size());
			m_DrawCommands.emplace_back(cmd);
		}
	}

	void VulkanMeshManager::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame)
	{
		ME_PROFILE_FUNCTION()

		int frameToUpdate{ static_cast<int>(frame - 1)};
		if (frameToUpdate < 0)
		{
			frameToUpdate = MAX_FRAMES_IN_FLIGHT - 1;
		}

		{
			ME_PROFILE_SCOPE("Mesh instance data update - buffer")

			memcpy(m_MeshInstanceDataBuffers[frameToUpdate].mapped, m_MeshInstanceData.data(), m_MeshInstanceData.size() * sizeof(MeshInstanceData));
		}

		{
			ME_PROFILE_SCOPE("Draw commands data update - buffer")

			memcpy(m_DrawCommandBuffers[frameToUpdate].mapped, m_DrawCommands.data(), m_DrawCommands.size() * sizeof(DrawCommand));
		}

		{
			ME_PROFILE_SCOPE("Mesh instance data update - descriptor sets")

			auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = m_MeshInstanceDataBuffers[frameToUpdate].buffer.buffer;
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

		{
			ME_PROFILE_SCOPE("Clearing the data")

			// not optimal, useful for testing - just rebuild all draw commands every frame and queue them
			m_DrawCommands.resize(0);
			m_MeshInstanceData.resize(0);

			m_BatchedDrawCommands.clear();
		}
	}

	void VulkanMeshManager::InitializeMeshInstanceDataBuffers()
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

	void VulkanMeshManager::InitializeMeshDataBuffers()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize constexpr BUFFER_SIZE{ sizeof(MeshData) * MAX_MESHES };

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_MeshDataBuffers.emplace_back(VulkanMappedBuffer{
												VulkanBuffer{BUFFER_SIZE,
																	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
												nullptr });

			// Persistent mapping
			vkMapMemory(deviceContext->GetLogicalDevice(), m_MeshDataBuffers[i].buffer.bufferMemory, 0, BUFFER_SIZE, 0, &m_MeshDataBuffers[i].mapped);
		}
	}

	void VulkanMeshManager::InitializeDrawCommandBuffers()
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

	void VulkanMeshManager::CreateVertexAndIndexBuffers()
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
