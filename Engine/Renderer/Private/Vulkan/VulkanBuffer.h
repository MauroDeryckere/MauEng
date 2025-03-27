#ifndef MAUREN_VULKANBUFFER_H
#define MAUREN_VULKANBUFFER_H

#include "RendererPCH.h"
#include "VulkanDeviceContext.h"
namespace MauRen
{
	// Must be manually destroyed by calling Destroy()! 
	struct VulkanBuffer final
	{
		VkBuffer buffer{ VK_NULL_HANDLE };
		VkDeviceMemory bufferMemory{ VK_NULL_HANDLE };
		VkDeviceSize size{ 0 };

		void Destroy();

		VulkanBuffer(VkDeviceSize deviceSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		VulkanBuffer() = default;

		~VulkanBuffer() = default;
	};

	struct VulkanMappedBuffer final
	{
		VulkanBuffer buffer{  };
		void* mapped{ nullptr };
	};

}

#endif