#include "VulkanSwapchainContext.h"
#include "VulkanSurfaceContext.h"
#include "VulkanDeviceContext.h"

namespace MauRen
{
	VulkanSwapchainContext::VulkanSwapchainContext(GLFWwindow* pWindow, VulkanSurfaceContext* pVulkanSurfaceContext, VulkanDeviceContext* pVulkanDeviceContext):
		m_pSurfaceContext{ pVulkanSurfaceContext },
		m_pDeviceContext{ pVulkanDeviceContext }

	{
		CreateSwapchain(pWindow);
		CreateImageViews();
	}

	VulkanSwapchainContext::~VulkanSwapchainContext()
	{
		for (auto const& imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_pDeviceContext->GetLogicalDevice(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(m_pDeviceContext->GetLogicalDevice(), m_SwapChain, nullptr);
	}

	SwapChainSupportDetails VulkanSwapchainContext::QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface)
	{
		SwapChainSupportDetails details;

		// basic surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, windowSurface, &details.capabilities);

		// querying the supported surface formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &formatCount, details.formats.data());
		}

		// querying the supported presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void VulkanSwapchainContext::CreateSwapchain(GLFWwindow* pWindow)
	{
		SwapChainSupportDetails const swapChainSupport{ QuerySwapchainSupport( m_pDeviceContext->GetPhysicalDevice(), m_pSurfaceContext->GetWindowSurface()) };

		VkSurfaceFormatKHR const surfaceFormat{ ChooseSwapSurfaceFormat(swapChainSupport.formats) };
		VkPresentModeKHR const presentMode{ ChooseSwapPresentMode(swapChainSupport.presentModes) };
		VkExtent2D const extent{ ChooseSwapExtent(pWindow, swapChainSupport.capabilities) };

		uint32_t imageCount{ swapChainSupport.capabilities.minImageCount + 1 };

		// Aim for triple buffering if possible
		if (imageCount < 3 && swapChainSupport.capabilities.maxImageCount >= 3)
		{
			imageCount = 3;
		}

		// 0 is a special value that means that there is no maximum
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_pSurfaceContext->GetWindowSurface();

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = m_pDeviceContext->FindQueueFamilies();
		uint32_t queueFamilyIndices[]{ indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (m_pDeviceContext->IsUsingUnifiedGraphicsPresentQueue())
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional

		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_pDeviceContext->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(m_pDeviceContext->GetLogicalDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_pDeviceContext->GetLogicalDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;

		std::cout << "Swapchain: " << imageCount << " images, Format: " << surfaceFormat.format
			<< ", Extent: " << extent.width << "x" << extent.height
			<< ", Present Mode: " << presentMode << std::endl;
	}

	void VulkanSwapchainContext::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());
		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_pDeviceContext->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	VkSurfaceFormatKHR VulkanSwapchainContext::ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats)
	{
		// For the color space we'll use SRGB if it is available.
		// Because it results in more accurate perceived colors.
		// It is also pretty much the standard color space for images.
		for (auto const& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}
		// If that also fails then we could start ranking the available formats based on how "good" they are.
		// In most cases it's okay to just settle with the first format that is specified.
		// TODO Implement ranking
		return availableFormats[0];
	}

	VkPresentModeKHR VulkanSwapchainContext::ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes)
	{
		for (auto const& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		// Guaruanteed to be available
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapchainContext::ChooseSwapExtent(GLFWwindow* pWindow, VkSurfaceCapabilitiesKHR const& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		int width, height;
		glfwGetFramebufferSize(pWindow, &width, &height);

		VkExtent2D actualExtent
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

}