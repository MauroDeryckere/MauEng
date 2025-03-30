#ifndef MAUREN_VULKANMESH_H
#define MAUREN_VULKANMESH_H

#include "RendererPCH.h"
#include "VulkanCommandPoolManager.h"
#include "Mesh.h"
#include "VulkanBuffer.h"

namespace MauRen
{
	class VulkanMesh final
	{
	public:
		struct MeshPushConstant final
		{
			glm::mat4 m_ModelMatrix{ 1.0f };

			// Material Data
			uint32_t m_AlbedoTextureID{ UINT32_MAX };
			uint32_t m_NormalTextureID{ UINT32_MAX };
			uint32_t m_RoughnessTextureID{ UINT32_MAX };
			uint32_t m_MetallicTextureID{ UINT32_MAX };
		};

		VulkanMesh(VulkanCommandPoolManager const& CmdPoolManager, Mesh const& mesh);
		~VulkanMesh() = default;

		void Destroy();
		void Draw(VkCommandBuffer commandBuffer) const;

		[[nodiscard]] MeshPushConstant const& GetPushConstant() const noexcept { return m_PushConstant; }
		[[nodiscard]] MeshPushConstant& GetPushConstant() noexcept { return m_PushConstant; }

		[[nodiscard]] uint32_t GetVertexCount() const noexcept { return m_VertexCount; }
		[[nodiscard]] uint32_t GetIndexCount() const noexcept { return m_IndexCount; }

		VulkanMesh(VulkanMesh const&) = default;
		VulkanMesh(VulkanMesh&&) = default;
		VulkanMesh& operator=(VulkanMesh const&) = default;
		VulkanMesh& operator=(VulkanMesh&&) = default;

	private:
		VulkanBuffer m_VertexBuffer{};
		VulkanBuffer m_IndexBuffer{};

		uint32_t m_VertexCount{ 0 };
		uint32_t m_IndexCount{ 0 };

		glm::mat4 m_ModelMatrix{ 1.0f };
		MeshPushConstant m_PushConstant{ };

		void CreateVertexBuffer(VulkanCommandPoolManager const& CmdPoolManager, std::vector<Vertex> const& vertices);
		void CreateIndexBuffer(VulkanCommandPoolManager const& CmdPoolManager, std::vector<uint32_t> const& indices);
	};
}

#endif