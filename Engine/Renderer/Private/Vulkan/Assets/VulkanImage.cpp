#include "VulkanImage.h"

#include "../VulkanCommandPoolManager.h"
#include "Vulkan/VulkanMemoryAllocator.h"

namespace MauRen
{
	void VulkanImage::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		DestroyAllImageViews();

		if (image != VK_NULL_HANDLE)
		{
			vmaDestroyImage(VulkanMemoryAllocator::GetInstance().GetAllocator(), image, alloc);
			image = VK_NULL_HANDLE;
			alloc = VK_NULL_HANDLE;
		}
	}

	void VulkanImage::TransitionImageLayout(VulkanCommandPoolManager const& CmdPoolManager, VkImageLayout newLayout, VkPipelineStageFlags2 dstStageMask,
		VkAccessFlags2 dstAccessMask)
	{
		ME_RENDERER_ASSERT(layout != newLayout);

		VkCommandBuffer const commandBuffer{ CmdPoolManager.BeginSingleTimeCommands() };
		TransitionImageLayout(commandBuffer, newLayout, dstStageMask, dstAccessMask);
		CmdPoolManager.EndSingleTimeCommands(commandBuffer);
	}

	void VulkanImage::TransitionImageLayout(VkCommandBuffer cmdBuffer,
		VkImageLayout newLayout,
		VkPipelineStageFlags2 dstStageMask,
		VkAccessFlags2 dstAccessMask)
	{
		ME_RENDERER_ASSERT(layout != newLayout);
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

		VkFormatProperties formatProperties{};
		vkGetPhysicalDeviceFormatProperties(deviceContext->GetPhysicalDevice(), format, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("Texture image format does not support linear blitting!");
		}

		int32_t mipWidth{ static_cast<int32_t>(width) };
		int32_t mipHeight{ static_cast<int32_t>(height) };

		for (uint32_t i = 1; i < mipLevels; ++i)
		{
			VkImageMemoryBarrier2 preBlitBarrier{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = image,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = i - 1,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			VkDependencyInfo depInfo{
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &preBlitBarrier
			};
			vkCmdPipelineBarrier2(commandBuffer, &depInfo);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = {
				mipWidth > 1 ? mipWidth / 2 : 1,
				mipHeight > 1 ? mipHeight / 2 : 1,
				1
			};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(
				commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR
			);

			// Transition the previous mip level to SHADER_READ_ONLY_OPTIMAL
			VkImageMemoryBarrier2 postBlitBarrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = image,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = i - 1,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			depInfo = VkDependencyInfo{
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &postBlitBarrier
			};
			vkCmdPipelineBarrier2(commandBuffer, &depInfo);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		// Transition last mip level to SHADER_READ_ONLY_OPTIMAL
		VkImageMemoryBarrier2 finalBarrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = mipLevels - 1,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		lastStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		lastAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

		VkDependencyInfo finalDepInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &finalBarrier
		};
		vkCmdPipelineBarrier2(commandBuffer, &finalDepInfo);

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
	                         uint32_t imgMipLevels, float memoryPriority)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		format = imgFormat;

		width = imgWidth;
		height = imgHeight;

		mipLevels = imgMipLevels;

		lastStage = VK_PIPELINE_STAGE_2_NONE;
		lastAccess = VK_ACCESS_2_NONE;
		memPriority = std::clamp(memoryPriority, 0.f, 1.f);

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

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VulkanMemoryAllocator::GetMemoryUsageFromVkProperties(properties);
		allocCreateInfo.priority = memPriority;

		if (vmaCreateImage(VulkanMemoryAllocator::GetInstance().GetAllocator(), &imageInfo, &allocCreateInfo, &image, &alloc, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image with VMA!");
		}
	}
}
