#ifndef MAUREN_VULKANMESHMANAGER_H
#define MAUREN_VULKANMESHMANAGER_H

#include "MeshInstance.h"
#include "RendererPCH.h"
#include "../VulkanBuffer.h"
#include "BindlessData.h"

namespace MauRen
{
	class VulkanDescriptorContext;
	class VulkanCommandPoolManager;

	class VulkanMeshManager final : public MauCor::Singleton<VulkanMeshManager>
	{
	public:
		bool Initialize(VulkanCommandPoolManager const * CmdPoolManager);
		bool Destroy();
		[[nodiscard]] std::pair<std::unordered_map<std::string, LoadedMeshes_PathInfo> const&, std::vector<MeshData>const&> GetLoadedMeshesPathMap() const noexcept { return { m_LoadedMeshes_Path, m_MeshData }; }

		void UnloadMesh(uint32_t meshID) noexcept;
		[[nodiscard]] uint32_t LoadMesh(char const* path, VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext) noexcept;
		[[nodiscard]] MeshData const& GetMeshData(uint32_t meshID) const;

		void QueueDraw(glm::mat4 const& transformMat, uint32_t meshID) noexcept
		{
			auto const it{ m_LoadedMeshes.find(meshID) };
			ME_ASSERT(it != end(m_LoadedMeshes));

			auto const& meshData{ m_MeshData[it->second] };

			for (uint32_t sub{ meshData.firstSubMesh }; sub < meshData.firstSubMesh + meshData.subMeshCount; ++ sub)
			{
				auto const& subMesh{ m_SubMeshes[sub] };

				m_MeshInstanceData.emplace_back(transformMat, sub, subMesh.materialID, meshData.flags);

				if (m_BatchedDrawCommands[sub] != INVALID_DRAW_COMMAND)
				{
					// Already added this mesh this frame; just increment instance count
					m_DrawCommands[m_BatchedDrawCommands[sub]].instanceCount++;
				}
				else
				{
					// First time seeing this mesh this frame; create a new draw command
					uint32_t const instanceOffset{ static_cast<uint32_t>(m_MeshInstanceData.size() - 1) };

					m_BatchedDrawCommands[sub] = static_cast<uint32_t>(m_DrawCommands.size());
					m_DrawCommands.emplace_back(subMesh.indexCount, 1, subMesh.firstIndex, subMesh.vertexOffset, instanceOffset);
				}
			}
		}

		void PreDraw(VulkanDescriptorContext& descriptorContext, uint32_t frame);
		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame);
		void PostDraw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t setCount, VkDescriptorSet const* pDescriptorSets, uint32_t frame);

		VulkanMeshManager(VulkanMeshManager const&) = delete;
		VulkanMeshManager(VulkanMeshManager&&) = delete;
		VulkanMeshManager& operator=(VulkanMeshManager const&) = delete;
		VulkanMeshManager& operator=(VulkanMeshManager const&&) = delete;

	private:
		friend class MauCor::Singleton<VulkanMeshManager>;
		VulkanMeshManager() = default;
		virtual ~VulkanMeshManager() override = default;

		VulkanCommandPoolManager const* m_CmdPoolManager{ nullptr };

		// 1:1 copy w/ GPU buffers
		std::vector<MeshInstanceData> m_MeshInstanceData;
		std::vector<VulkanMappedBuffer> m_MeshInstanceDataBuffers;

		// Data for each mesh
		std::vector<MeshData> m_MeshData;
		std::vector<SubMeshData> m_SubMeshes;

		// 1:1 copy w/ GPU buffers
		std::vector<DrawCommand> m_DrawCommands;
		std::vector<VulkanMappedBuffer> m_DrawCommandBuffers;

		// All vertices in one big buffer
		std::vector<VulkanMappedBuffer> m_VertexBuffer;
		// All indices in one big buffer
		std::vector<VulkanMappedBuffer> m_IndexBuffer;

		// Maps SubMeshID -> index into m_DrawCommands
		// DrawCommands[SubMeshID] == uint max -> no batch yet; else it's the idx into the vec
		std::vector<uint32_t> m_BatchedDrawCommands;

		// maps mesh ID -> index into m_MeshData
		std::unordered_map<uint32_t, uint32_t> m_LoadedMeshes;

		// map model/mesh path into m_MeshData
		std::unordered_map<std::string, LoadedMeshes_PathInfo> m_LoadedMeshes_Path;

		std::unordered_map<uint32_t, std::string> m_MeshID_path;

		uint32_t m_CurrentVertexOffset{ 0 }; // current vertex offset in the "global" vertex buffer
		uint32_t m_CurrentIndexOffset{ 0 }; // current index offset in the "global" index buffer
		uint32_t m_NextID{ 0 }; // next available mesh ID

		struct PendingMeshUpload final
		{
			uint32_t meshID;
		};

		struct CPUModelDataCache final
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			uint32_t vertexOffset;
			uint32_t indexOffset;

			uint32_t uses;
		};
		std::unordered_map<uint32_t, CPUModelDataCache> m_CPUModelData;

		std::vector<PendingMeshUpload> m_PendingUploads[MAX_FRAMES_IN_FLIGHT];

		struct FreeRange
		{
			uint32_t offset;
			uint32_t size;

			uint32_t End() const { return offset + size; }

			bool operator<(FreeRange const& other) const
			{
				return offset < other.offset;
			}

			static void MergeFreeRanges(std::vector<FreeRange>& ranges)
			{
				if (ranges.empty())
					return;

				std::sort(ranges.begin(), ranges.end());

				std::vector<FreeRange> merged;
				merged.reserve(ranges.size());
				merged.push_back(ranges[0]);

				for (size_t i{ 1 }; i < ranges.size(); ++i)
				{
					FreeRange& last{ merged.back() };
					FreeRange const& current{ ranges[i] };

					if (last.End() >= current.offset)
					{
						ME_RENDERER_ASSERT(last.End() == current.offset, "Overlapping free ranges detected!");
						last.size = std::max(last.End(), current.End()) - last.offset;
					}
					else
					{
						merged.push_back(current);
					}
				}

				ranges = std::move(merged);
			}
		};

		std::vector<FreeRange> m_FreeIndices{};
		std::vector<FreeRange> m_FreeVertices{};

		void InitializeMeshInstanceDataBuffers() noexcept;
		void InitializeDrawCommandBuffers() noexcept;

		void CreateVertexAndIndexBuffers() noexcept;
	};
}

#endif