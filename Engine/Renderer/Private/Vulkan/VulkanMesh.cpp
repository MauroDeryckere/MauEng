#include "VulkanMesh.h"

namespace MauRen
{
	VulkanMesh::VulkanMesh(VulkanCommandPoolManager const& CmdPoolManager, Mesh const& mesh)
	{
		auto const& indices{ mesh.GetIndices() };
		auto const& vertices{ mesh.GetVertices() };

		m_IndexCount = static_cast<uint32_t>(indices.size());
		m_VertexCount = static_cast<uint32_t>(vertices.size());

		CreateVertexBuffer(CmdPoolManager, vertices);
		CreateIndexBuffer(CmdPoolManager, indices);
	}

	void VulkanMesh::Destroy()
	{
		m_IndexBuffer.Destroy();
		m_VertexBuffer.Destroy();
	}

	void VulkanMesh::Draw(VkCommandBuffer commandBuffer) const
	{
		VkBuffer vertexBuffers[] = { m_VertexBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		// Bind the index buffer
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void VulkanMesh::CreateVertexBuffer(VulkanCommandPoolManager const& CmdPoolManager, std::vector<Vertex> const& vertices)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize const bufferSize{ sizeof(vertices[0]) * vertices.size() };

		VulkanBuffer stagingBuffer{ bufferSize,
									VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		if (stagingBuffer.buffer == VK_NULL_HANDLE || stagingBuffer.bufferMemory == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to create staging buffer.");
		}


		void* data;
		if (vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, bufferSize, 0, &data) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to map Vulkan buffer memory.");
		}

		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);

		m_VertexBuffer = { bufferSize,
							VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

		VulkanBuffer::CopyBuffer(CmdPoolManager, stagingBuffer.buffer, m_VertexBuffer.buffer, bufferSize);

		stagingBuffer.Destroy();
	}

	void VulkanMesh::CreateIndexBuffer(VulkanCommandPoolManager const& CmdPoolManager, std::vector<uint32_t> const& indices)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkDeviceSize const bufferSize{ sizeof(indices[0]) * indices.size() };

		VulkanBuffer stagingBuffer{ bufferSize,
									VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		void* data;
		vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);


		m_IndexBuffer = { bufferSize,
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

		VulkanBuffer::CopyBuffer(CmdPoolManager, stagingBuffer.buffer, m_IndexBuffer.buffer, bufferSize);

		stagingBuffer.Destroy();
	}
}
