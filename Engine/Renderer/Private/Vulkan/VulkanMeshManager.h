#ifndef MAUREN_VULKANMESHMANAGER_H
#define MAUREN_VULKANMESHMANAGER_H

#include "RendererPCH.h"
#include "VulkanMesh.h"

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

		[[nodiscard]] VulkanMesh const& GetVulkanMesh(uint32_t meshID) const;

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

		// Old Setup

		uint32_t m_NextID{ 0 };
		std::unordered_map<uint32_t, VulkanMesh> m_Meshes;

		std::unordered_map<uint32_t, std::vector<MeshInstance>> m_MeshBatches;

		// Bindless Setup
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

		void InitializeMeshInstanceDataBuffers();
		void InitializeMeshDataBuffers();
		void InitializeDrawCommandBuffers();

		void CreateVertexAndIndexBuffers();
	};
}

#endif