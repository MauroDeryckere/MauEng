#include "VulkanBuffer.h"

#include "VulkanCommandPoolManager.h"

namespace MauRen
{
	void VulkanBuffer::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), buffer, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), bufferMemory, nullptr);
	}

	VulkanBuffer::VulkanBuffer(VkDeviceSize deviceSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		if (deviceSize == 0)
		{
			return;
		}

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		size = deviceSize;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(deviceContext->GetLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(deviceContext->GetLogicalDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		//allocInfo.allocationSize = size;
		allocInfo.memoryTypeIndex = VulkanUtils::FindMemoryType(deviceContext->GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

		//TODO
		/*
		 * It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory for every individual buffer.
		 * The maximum number of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an NVIDIA GTX 1080.
		 * The right way to allocate memory for a large number of objects at the same time is to create a custom allocator that splits up a single allocation among many different objects by using the offset parameters that we've seen in many functions.

		 * You can either implement such an allocator yourself, or use the VulkanMemoryAllocator library provided by the GPUOpen initiative. // https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
		 * However, for this tutorial it's okay to use a separate allocation for every resource, because we won't come close to hitting any of these limits for now.
		 */

		if (vkAllocateMemory(deviceContext->GetLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory!");
		}

		vkBindBufferMemory(deviceContext->GetLogicalDevice(), buffer, bufferMemory, 0);
	}

	void VulkanBuffer::CopyBuffer(VulkanCommandPoolManager const& CmPoolManager, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		// TODO
		/* Memory transfer operations are executed using command buffers, just like drawing commands.
		 * Therefore we must first allocate a temporary command buffer.
		 * You may wish to create a separate command pool for these kinds of short-lived buffers, because the implementation may be able to apply memory allocation optimizations.
		 * You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case.
		 */

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
