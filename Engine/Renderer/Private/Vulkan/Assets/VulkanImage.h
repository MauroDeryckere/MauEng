#ifndef MAUREN_VULKANIMAGE_H
#define MAUREN_VULKANIMAGE_H

#include "RendererPCH.h"

namespace MauRen
{
	class VulkanCommandPoolManager;

	//TODO separate class refactor in future
	struct VulkanImage final
	{
		VkImage image{ VK_NULL_HANDLE };
		VkDeviceMemory imageMemory{ VK_NULL_HANDLE };
		VkFormat format{ VK_FORMAT_UNDEFINED };
		VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };

		// Currently only work with one view but may support multiple later
		std::vector<VkImageView> imageViews{ };

		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint32_t mipLevels{ 1 };
		uint32_t arrayLayers{ 1 };
		void Destroy();

		// Transition image layout using single time commands 
		void TransitionImageLayout(VulkanCommandPoolManager const& CmdPoolManager, VkImageLayout newLayout);
		// Transition image layout using pre-existing command buffer & memeory bariers.
		void TransitionImageLayout( VkCommandBuffer cmdBuffer, 
									VkImageLayout newLayout, 
									VkPipelineStageFlags2 srcStageMask,
									VkPipelineStageFlags2 dstStageMask,
									VkAccessFlags2 srcAccessMask,
									VkAccessFlags2 dstAccessMask);

		void GenerateMipmaps(VulkanCommandPoolManager const& CmdPoolManager);
		uint32_t CreateImageView(VkImageAspectFlags aspectFlags, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);

		void DestroyAllImageViews() noexcept;

		[[nodiscard]] bool Valid() const noexcept
		{
			return  VK_NULL_HANDLE != image
				and VK_NULL_HANDLE != imageMemory;
		}

		// Manual creation if necessary
		VulkanImage() = default;
		// Fully create image via the constructor
		VulkanImage(VkFormat imgFormat, 
					VkImageTiling tiling, 
					VkImageUsageFlags usage, 
					VkMemoryPropertyFlags properties,
					VkSampleCountFlagBits numSamples, 
					uint32_t imgWidth, 
					uint32_t imgHeight, 
					uint32_t imgMipLevels = 1,
					uint32_t arrLayers = 1,
					VkImageCreateFlags flags = 0);

		~VulkanImage() = default;

		VulkanImage(VulkanImage const&) = default;
		VulkanImage(VulkanImage&&) = default;
		VulkanImage& operator=(VulkanImage const&) = default;
		VulkanImage& operator=(VulkanImage&&) = default;

	};
}

#endif