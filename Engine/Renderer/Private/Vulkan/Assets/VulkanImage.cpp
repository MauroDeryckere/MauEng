#include "VulkanImage.h"

#include "../VulkanCommandPoolManager.h"

namespace MauRen
{
	void VulkanImage::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		DestroyAllImageViews();

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), image, nullptr);
		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), imageMemory, nullptr);
	}

	void VulkanImage::TransitionImageLayout(VulkanCommandPoolManager const& CmdPoolManager, VkImageLayout newLayout)
	{
		VkCommandBuffer const commandBuffer{ CmdPoolManager.BeginSingleTimeCommands() };

		VkPipelineStageFlags2 dstStageMask{ 0 };
		VkAccessFlags2 dstAccessMask{ 0 };

		if (layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		//else
		//{
		//	throw std::invalid_argument("Unsupported layout transition!");
		//}

		TransitionImageLayout(commandBuffer, newLayout, dstStageMask, dstAccessMask);

		CmdPoolManager.EndSingleTimeCommands(commandBuffer);
	}

	void VulkanImage::TransitionImageLayout(VkCommandBuffer cmdBuffer,
		VkImageLayout newLayout,
		VkPipelineStageFlags2 dstStageMask,
		VkAccessFlags2 dstAccessMask)
	{
		//ME_ASSERT(layout != newLayout);
		ME_ASSERT(cmdBuffer != VK_NULL_HANDLE);
		ME_ASSERT(image != VK_NULL_HANDLE);

		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.oldLayout = layout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		if (format == VK_FORMAT_D32_SFLOAT)
		{
			ME_ASSERT(1 == mipLevels);
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.srcAccessMask = lastAccess;
		barrier.dstAccessMask = dstAccessMask;

		barrier.srcStageMask = lastStage;
		barrier.dstStageMask = dstStageMask;

		layout = newLayout;

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers = &barrier;

		// Check if cmd pipeline barrier 2 is setup correctly
		//auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		//auto test = (PFN_vkCmdPipelineBarrier2)vkGetDeviceProcAddr(deviceContext->GetLogicalDevice(), "vkCmdPipelineBarrier2");
		//ME_ASSERT(test != nullptr);


		vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

		lastStage = dstStageMask;
		lastAccess = dstAccessMask;
	}

	void VulkanImage::GenerateMipmaps(VulkanCommandPoolManager const& CmdPoolManager)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkCommandBuffer const commandBuffer{ CmdPoolManager.BeginSingleTimeCommands() };

		// TODO
		/*
		 *There are two alternatives in this case.
		 *You could implement a function that searches common texture image formats for one that does support linear blitting,
		 *or you could implement the mipmap generation in software with a library like stb_image_resize.
		 *Each mip level can then be loaded into the image in the same way that you loaded the original image.
		 *
		 *It should be noted that it is uncommon in practice to generate the mipmap levels at runtime anyway.
		 *Usually they are pregenerated and stored in the texture file alongside the base level to improve loading speed.
		 *Implementing resizing in software and loading multiple levels from a file is left as an exercise to the reader.
		 **/

		 // Check if image format supports linear blitting
		VkFormatProperties formatProperties{};
		vkGetPhysicalDeviceFormatProperties(deviceContext->GetPhysicalDevice(), format, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("Texture image format does not support linear blitting!");
		}

		// We're going to make several transitions, so we'll reuse this VkImageMemoryBarrier
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth{ static_cast<int32_t>(width) };
		int32_t mipHeight{ static_cast<int32_t>(height) };

		for (uint32_t i{ 1 }; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;


			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1)
			{
				mipWidth /= 2;
			}
			if (mipHeight > 1)
			{
				mipHeight /= 2;
			}
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		CmdPoolManager.EndSingleTimeCommands(commandBuffer);
	}

	uint32_t VulkanImage::CreateImageView(VkImageAspectFlags aspectFlags)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView{};
		if (vkCreateImageView(deviceContext->GetLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture image view!");
		}

		imageViews.emplace_back(imageView);

		return static_cast<uint32_t>(imageViews.size() - 1);
	}

	void VulkanImage::DestroyAllImageViews() noexcept
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		for (auto& imageView : imageViews)
		{
			VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), imageView, nullptr);
		}
	}

	VulkanImage::VulkanImage(VkFormat imgFormat, VkImageTiling tiling, VkImageUsageFlags usage,
	                         VkMemoryPropertyFlags properties, VkSampleCountFlagBits numSamples, uint32_t imgWidth, uint32_t imgHeight,
	                         uint32_t imgMipLevels)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		format = imgFormat;

		width = imgWidth;
		height = imgHeight;

		mipLevels = imgMipLevels;

		lastStage = VK_PIPELINE_STAGE_2_NONE;
		lastAccess = VK_ACCESS_2_NONE;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		layout = VK_IMAGE_LAYOUT_UNDEFINED;

		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;
		imageInfo.flags = 0; // Optional

		if (vkCreateImage(deviceContext->GetLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image!");
		}

		VkMemoryRequirements memRequirements{};
		vkGetImageMemoryRequirements(deviceContext->GetLogicalDevice(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanUtils::FindMemoryType(deviceContext->GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(deviceContext->GetLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory!");
		}

		vkBindImageMemory(deviceContext->GetLogicalDevice(), image, imageMemory, 0);
	}
}
