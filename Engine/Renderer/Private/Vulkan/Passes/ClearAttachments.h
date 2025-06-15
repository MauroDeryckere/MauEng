#ifndef MAUREN_CLEARATTACHMENTS_H
#define MAUREN_CLEARATTACHMENTS_H

#include "RendererPCH.h"
#include "Vulkan/Assets/VulkanImage.h"
namespace MauRen
{
	inline void ClearAttachments(
		VkCommandBuffer commandBuffer,
		std::vector<VulkanImage> const& colors,
		std::vector<VkClearValue> const& colorClearValues,
		VulkanImage depth = {},
		VkClearValue depthClearValue = {},
		VkExtent2D extent  = {})
	{
		std::vector<VkRenderingAttachmentInfo> colorAttachments;
		for (auto& img : colors)
		{
			ME_RENDERER_ASSERT(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == img.layout);

			VkRenderingAttachmentInfo attachment{};
			attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			attachment.imageView = img.imageViews[0];
			attachment.imageLayout = img.layout;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachments.emplace_back(attachment);
		}

		VkRenderingAttachmentInfo depthAttachment{};
		if (depth.image != VK_NULL_HANDLE)
		{
			ME_RENDERER_ASSERT(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL == depth.layout);

			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageView = depth.imageViews[0];
			depthAttachment.imageLayout = depth.layout;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		}

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea.offset = { 0, 0 };
		renderingInfo.renderArea.extent = extent;
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
		renderingInfo.pColorAttachments = ((not colorAttachments.empty()) ? colorAttachments.data() : nullptr);
		renderingInfo.pDepthAttachment = ((depth.image != VK_NULL_HANDLE) ? &depthAttachment : nullptr);

		vkCmdBeginRendering(commandBuffer, &renderingInfo);
		std::vector<VkClearAttachment> clearAttachments;

		for (uint32_t i{ 0 }; i < colors.size(); ++i)
		{
			VkClearAttachment clear{};
			clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			clear.colorAttachment = i;
			clear.clearValue = colorClearValues[i];
			clearAttachments.emplace_back(clear);
		}
		if (depth.image != VK_NULL_HANDLE)
		{
			VkClearAttachment clear{};
			clear.aspectMask = 0;
			clear.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
			clear.clearValue = depthClearValue;
			clearAttachments.emplace_back(clear);
		}

		VkClearRect clearRect{};
		clearRect.rect.offset = { 0, 0 };
		clearRect.rect.extent = extent;
		clearRect.baseArrayLayer = 0;
		clearRect.layerCount = 1;
		vkCmdClearAttachments(commandBuffer, static_cast<uint32_t>(clearAttachments.size()), clearAttachments.data(), 1, &clearRect);
		vkCmdEndRendering(commandBuffer);

	}
}

#endif