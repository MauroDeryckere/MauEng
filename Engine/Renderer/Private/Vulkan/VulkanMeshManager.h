#ifndef MAUREN_VULKANMESHMANAGER_H
#define MAUREN_VULKANMESHMANAGER_H

#include "MeshInstance.h"
#include "RendererPCH.h"
#include "Mesh.h"
#include "VulkanBuffer.h"
#include "Bindless/BindlessData.h"

namespace MauRen
{
	class VulkanDescriptorContext;
	class VulkanCommandPoolManager;

	class VulkanMeshManager final : public MauCor::Singleton<VulkanMeshManager>
	{
	public:
		bool Initialize(VulkanCommandPoolManager const * CmdPoolManager);
		bool Destroy();

		MeshInstance LoadMesh(char const* path, VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext);

		[[nodiscard]] MeshData const& GetMesh(uint32_t meshID) const;

		__forceinline void QueueDraw(glm::mat4 const& transformMat, uint32_t meshID, uint32_t materialID)
		{
			m_MeshInstanceData.emplace_back(transformMat, meshID, materialID, 0, 0);

			if (m_BatchedDrawCommands[meshID] != UINT32_MAX)
			{
				// Already added this mesh this frame; just increment instance count
				m_DrawCommands[m_BatchedDrawCommands[meshID]].instanceCount++;
			}
			else
			{
				// First time seeing this mesh this frame; create a new draw command
				const MeshData& mesh{ m_MeshData[m_LoadedMeshes.at(meshID)] };
				uint32_t const instanceOffset{ static_cast<uint32_t>(m_MeshInstanceData.size() - 1) };

				m_BatchedDrawCommands[meshID] = static_cast<uint32_t>(m_DrawCommands.size());
				m_DrawCommands.emplace_back(mesh.indexCount, 1, mesh.firstIndex, mesh.vertexOffset, instanceOffset);
			}
		}

		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame);

		VulkanMeshManager(VulkanMeshManager const&) = delete;
		VulkanMeshManager(VulkanMeshManager&&) = delete;
		VulkanMeshManager& operator=(VulkanMeshManager const&) = delete;
		VulkanMeshManager& operator=(VulkanMeshManager const&&) = delete;

	private:
		friend class MauCor::Singleton<VulkanMeshManager>;
		VulkanMeshManager() = default;
		virtual ~VulkanMeshManager() override = default;

		VulkanCommandPoolManager const* m_CmdPoolManager;

		// 1:1 copy w/ GPU buffers
		std::vector<MeshInstanceData> m_MeshInstanceData;
		std::vector<VulkanMappedBuffer> m_MeshInstanceDataBuffers;

		std::vector<MeshData> m_MeshData;

		// 1:1 copy w/ GPU buffers
		std::vector<DrawCommand> m_DrawCommands;
		std::vector<VulkanMappedBuffer> m_DrawCommandBuffers;

		// All vertices in one big buffer
		VulkanMappedBuffer m_VertexBuffer;
		// All indices in one big buffer
		VulkanMappedBuffer m_IndexBuffer;

		// maps mesh ID -> index into m_DrawCommands
		// DrawCommands[MeshId] == uint max -> no batch yet; else it's the idx into the vec
		std::vector<uint32_t> m_BatchedDrawCommands;

		// maps mesh ID -> index into m_MeshData
		std::unordered_map<uint32_t, uint32_t> m_LoadedMeshes;
		// map path into m_MeshData
		std::unordered_map<char const*, uint32_t> m_LoadedMeshes_Path;

		uint32_t m_CurrentVertexOffset{ 0 };
		uint32_t m_CurrentIndexOffset{ 0 };
		uint32_t m_NextID{ 0 };

		void InitializeMeshInstanceDataBuffers();
		void InitializeDrawCommandBuffers();

		void CreateVertexAndIndexBuffers();
	};
}

#endif