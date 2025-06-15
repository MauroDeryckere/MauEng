#include "VulkanBuffer.h"

#include "VulkanCommandPoolManager.h"
#include "VulkanMemoryAllocator.h"

namespace MauRen
{
	void VulkanBuffer::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		
		auto allocator{ VulkanMemoryAllocator::GetInstance().GetAllocator() };
		vmaDestroyBuffer(allocator, buffer, alloc);

		buffer = VK_NULL_HANDLE;
		alloc = nullptr;
	}

	VulkanBuffer::VulkanBuffer(VkDeviceSize deviceSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, float memoryPriority)
	{
		if (deviceSize == 0)
		{
			return;
		}

		auto allocator{ VulkanMemoryAllocator::GetInstance().GetAllocator() };
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		size = deviceSize;
		memPriority = memoryPriority;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VulkanMemoryAllocator::GetMemoryUsageFromVkProperties(properties);
		allocCreateInfo.priority = memPriority;

		VmaAllocation allocation;
		if (vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer!");
		}

		alloc = allocation;
	}

	void VulkanMappedBuffer::UnMap()
	{
		if (nullptr != mapped)
		{
			vmaUnmapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), buffer.alloc);
		}
		mapped = nullptr;
	}

	void VulkanBuffer::CopyBuffer(VulkanCommandPoolManager const& CmPoolManager, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer{ CmPoolManager.BeginSingleTimeCommands() };

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		CmPoolManager.EndSingleTimeCommands(commandBuffer);
	}

	void VulkanBuffer::CopyBufferToImage(VulkanCommandPoolManager const& CmPoolManager, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer{ CmPoolManager.BeginSingleTimeCommands() };

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);

		CmPoolManager.EndSingleTimeCommands(commandBuffer);
	}

}
