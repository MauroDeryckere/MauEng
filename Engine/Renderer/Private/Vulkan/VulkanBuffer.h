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
		VkDeviceSize size{ 0 };
		float memPriority{ 0.f };

		VkBufferUsageFlags _usage{};
		VkMemoryPropertyFlags _properties{};

		VmaAllocation alloc{ nullptr };

		void Destroy();

		void Resize(VkDeviceSize newSize,
					VkBufferUsageFlags usage,
					VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					float memoryPriority = 0.f);

		static void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
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

		void UnMap();
	};

}

#endif