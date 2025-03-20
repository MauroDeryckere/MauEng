#ifndef MAUREN_VULKANDEVICECONTEXT_H
#define MAUREN_VULKANDEVICECONTEXT_H

#include "RendererPCH.h"

namespace MauRen
{
	struct VulkanDeviceContext final
	{
		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		VkDevice logicalDevice{ VK_NULL_HANDLE };

		// Functions to deal with device creation
	};
}

#endif // MAUREN_VULKANDEVICECONTEXT_H