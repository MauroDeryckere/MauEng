#ifndef MAUREN_VULKANMESHMANAGER_H
#define MAUREN_VULKANMESHMANAGER_H

#include "RendererPCH.h"
#include "Mesh.h"
#include "VulkanBuffer.h"
#include "Bindless/BindlessData.h"

namespace MauRen
{
	class VulkanCommandPoolManager;

	class VulkanMeshManager final : public MauCor::Singleton<VulkanMeshManager>
	{
	public:
		bool Initialize(VulkanCommandPoolManager const * CmdPoolManager);
		bool Destroy();

		void LoadMesh(Mesh& mesh);

		[[nodiscard]] MeshData const& GetMesh(uint32_t meshID) const;

		void QueueDraw(MeshInstance const* instance);

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
		std::vector<MeshData> m_MeshData;

		std::vector<VulkanMappedBuffer> m_MeshInstanceDataBuffers;
		std::vector<VulkanMappedBuffer> m_MeshDataBuffers;

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


		uint32_t m_CurrentVertexOffset{ 0 };
		uint32_t m_CurrentIndexOffset{ 0 };
		uint32_t m_NextID{ 0 };

		void InitializeMeshInstanceDataBuffers();
		void InitializeMeshDataBuffers();
		void InitializeDrawCommandBuffers();

		void CreateVertexAndIndexBuffers();
	};
}

#endif