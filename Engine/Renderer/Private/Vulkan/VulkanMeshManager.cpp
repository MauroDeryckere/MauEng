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

		// Old setup
		for (auto& m : m_Meshes)
		{
			m.second.Destroy();
		}

		return true;
	}

	void VulkanMeshManager::LoadMesh(Mesh& mesh)
	{
		ME_PROFILE_FUNCTION()

			// OLD LOGIC
			//auto const it{ m_Meshes.find(mesh.GetMeshID()) };
			//if (it != end(m_Meshes))
			//{
			//	return;
			//}

			//mesh.SetMeshID(m_NextID);
			//m_Meshes.emplace(m_NextID, VulkanMesh{ *m_CmdPoolManager, mesh });

		mesh.SetMeshID(m_NextID);
		m_MeshData.emplace_back(mesh.GetIndices().size());



		m_NextID++;
	}

	const VulkanMesh& VulkanMeshManager::GetVulkanMesh(uint32_t meshID) const
	{
		auto const it{ m_Meshes.find(meshID) };
		if (it != m_Meshes.end())
		{
			return it->second;
		}

		assert(false && "Mesh not found in VulkanMeshManager");

		throw std::runtime_error("");
	}

	void VulkanMeshManager::QueueDraw(MeshInstance const* instance)
	{
		//m_MeshBatches[instance->GetMeshID()].emplace_back(*instance);

		//TODO

		//instance->GetMeshID()
		m_DrawCommands.emplace_back();
	}

	void VulkanMeshManager::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame)
	{
		ME_PROFILE_FUNCTION()

		// OLD APPROACH

		//// Draw each batch for each mesh
		//// TODO actually batch them
		//for (auto const& [meshID, instances] : m_MeshBatches)
		//{
		//	VulkanMesh const& mesh = m_Meshes.at(meshID);

		//	std::vector<glm::mat4> modelMatrices;
		//	for (auto const& instance : instances)
		//	{
		//		MeshPushConstant mPush{ };
		//		mPush.m_ModelMatrix = instance.GetModelMatrix();

		//		//auto const& mat{ VulkanMaterialManager::GetInstance().GetMaterial() };
		//		ME_RENDERER_ASSERT(VulkanMaterialManager::GetInstance().Exists(instance.GetMaterialID()));

		//		mPush.m_MaterialID = instance.GetMaterialID();

		//		vkCmdPushConstants(commandBuffer, layout,
		//							VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mPush),
		//							&mPush);

		//		mesh.Draw(commandBuffer);

		//		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, setCount, pDescriptorSets, 0, nullptr);
		//		vkCmdDrawIndexed(commandBuffer, mesh.GetIndexCount(), 1, 0, 0, 0);
		//	}
		//}

		//m_MeshBatches.clear();


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

		// not optimal, useful for testing - just rebuild all draw commands every frame and queue them
		m_DrawCommands.clear();
	}

	void VulkanMeshManager::InitializeMeshInstanceDataBuffers()
	{
		auto deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

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
		auto deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

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
		auto deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

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
		auto deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

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
