#ifndef MAUREN_VULKANBUFFER_H
#define MAUREN_VULKANBUFFER_H

#include "RendererPCH.h"
#include "VulkanDeviceContext.h"
namespace MauRen
{
	class VulkanCommandPoolManager;

	// Must be manually destroyed by calling Destroy()! 
	struct VulkanBuffer final
	{
		VkBuffer buffer{ VK_NULL_HANDLE };
		VkDeviceMemory bufferMemory{ VK_NULL_HANDLE };
		VkDeviceSize size{ 0 };

		void Destroy();
		static void CopyBuffer(VulkanCommandPoolManager const& CmPoolManager, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		VulkanBuffer(VkDeviceSize deviceSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		VulkanBuffer() = default;
		~VulkanBuffer() = default;

		VulkanBuffer(VulkanBuffer const&) = default;
		VulkanBuffer(VulkanBuffer&&) = default;
		VulkanBuffer& operator=(VulkanBuffer const&) = default;
		VulkanBuffer& operator=(VulkanBuffer&&) = default;
	};

	struct VulkanMappedBuffer final
	{
		VulkanBuffer buffer{  };
		void* mapped{ nullptr };
	};

}

#endif