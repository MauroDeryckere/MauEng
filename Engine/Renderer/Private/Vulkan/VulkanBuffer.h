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
		float memPriority{ 0.f };
		void Destroy();
		static void CopyBuffer(VulkanCommandPoolManager const& CmPoolManager, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		static void CopyBufferToImage(VulkanCommandPoolManager const& CmPoolManager, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


		VulkanBuffer(VkDeviceSize deviceSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, float memoryPriority = 0.f);
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