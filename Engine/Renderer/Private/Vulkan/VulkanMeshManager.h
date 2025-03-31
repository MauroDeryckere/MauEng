#ifndef MAUREN_VULKANMESHMANAGER_H
#define MAUREN_VULKANMESHMANAGER_H

#include "RendererPCH.h"
#include "VulkanMesh.h"


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

		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets);

		VulkanMeshManager(VulkanMeshManager const&) = delete;
		VulkanMeshManager(VulkanMeshManager&&) = delete;
		VulkanMeshManager& operator=(VulkanMeshManager const&) = delete;
		VulkanMeshManager& operator=(VulkanMeshManager const&&) = delete;

	private:
		friend class MauCor::Singleton<VulkanMeshManager>;
		VulkanMeshManager() = default;
		virtual ~VulkanMeshManager() override = default;

		VulkanCommandPoolManager const* m_CmdPoolManager;

		uint32_t m_NextID{ 0 };
		std::unordered_map<uint32_t, VulkanMesh> m_Meshes;

		std::unordered_map<uint32_t, std::vector<MeshInstance>> m_MeshBatches;
	};
}

#endif