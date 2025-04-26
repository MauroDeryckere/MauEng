#include "RendererPCH.h"

#include "VulkanSwapchainContext.h"
#include "VulkanSurfaceContext.h"
#include "VulkanDeviceContext.h"
#include "VulkanGraphicsPipeline.h"

namespace MauRen
{
	void VulkanSwapchainContext::Initialize(SDL_Window* pWindow, VulkanSurfaceContext const * pVulkanSurfaceContext)
	{
		CreateSwapchain(pWindow, pVulkanSurfaceContext);
		CreateImageViews();
	}

	void VulkanSwapchainContext::InitializeResourcesAndCreateFrames(VulkanGraphicsPipeline const* pGraphicsPipeline)
	{
		CreateColorResources();
		CreateDepthResources();
	//	CreateFrameBuffers(pGraphicsPipeline);
	}

	void VulkanSwapchainContext::ReCreate(SDL_Window* pWindow, VulkanGraphicsPipeline const* pGraphicsPipeline, VulkanSurfaceContext const* pVulkanSurfaceContext)
	{
		Destroy();

		CreateSwapchain(pWindow, pVulkanSurfaceContext);
		CreateImageViews();
		CreateColorResources();
		CreateDepthResources();
	//	CreateFrameBuffers(pGraphicsPipeline);
	}

	void VulkanSwapchainContext::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		for (auto& framebuffer : m_SwapChainFrameBuffers)
		{
			VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), framebuffer, nullptr);
		}

		m_DepthImage.Destroy();
		m_ColorImage.Destroy();

		for (auto& image : m_SwapChainImages)
		{
			image.DestroyAllImageViews();
		}
		m_SwapChainImages.clear();

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_SwapChain, nullptr);
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

	VkFramebuffer VulkanSwapchainContext::GetSwapchainFrameBuffer(uint32_t imageIndex) const noexcept
	{
		assert(imageIndex < m_SwapChainFrameBuffers.size());
		return m_SwapChainFrameBuffers[imageIndex];
	}

	void VulkanSwapchainContext::CreateSwapchain(SDL_Window* pWindow, VulkanSurfaceContext const * pVulkanSurfaceContext)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		SwapChainSupportDetails const swapChainSupport{ QuerySwapchainSupport(deviceContext->GetPhysicalDevice(), pVulkanSurfaceContext->GetWindowSurface()) };

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
		createInfo.surface = pVulkanSurfaceContext->GetWindowSurface();

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = deviceContext->FindQueueFamilies();
		uint32_t queueFamilyIndices[]{ indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (deviceContext->IsUsingUnifiedGraphicsPresentQueue())
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

		if (vkCreateSwapchainKHR(deviceContext->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(deviceContext->GetLogicalDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		std::vector<VkImage> vkImages(imageCount);

		vkGetSwapchainImagesKHR(deviceContext->GetLogicalDevice(), m_SwapChain, &imageCount, vkImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;

		for (size_t i{ 0 }; i < imageCount; ++i)
		{
			m_SwapChainImages[i].image = vkImages[i];
			m_SwapChainImages[i].format = m_SwapChainImageFormat;
			m_SwapChainImages[i].width = m_SwapChainExtent.width;
			m_SwapChainImages[i].height = m_SwapChainExtent.height;
		}

		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Swapchain: {} images, Format: {}, Extent: {} x {} y, Present Mode: {}", imageCount, static_cast<int>(surfaceFormat.format), extent.width, extent.height, static_cast<int>(presentMode));
	}

	void VulkanSwapchainContext::CreateImageViews()
	{
		for (size_t i{ 0 }; i < m_SwapChainImages.size(); ++i)
		{
			m_SwapChainImages[i].CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
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

	VkExtent2D VulkanSwapchainContext::ChooseSwapExtent(SDL_Window* pWindow, VkSurfaceCapabilitiesKHR const& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		int width, height;
	//	glfwGetFramebufferSize(pWindow, &width, &height);
		SDL_GetWindowSize(pWindow, &width, &height);
		VkExtent2D actualExtent
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	void VulkanSwapchainContext::CreateColorResources()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkFormat const colorFormat{ GetImageFormat() };

		m_ColorImage = VulkanImage
		{
			colorFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			deviceContext->GetSampleCount(),
			GetExtent().width,
			GetExtent().height
		};

		m_ColorImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void VulkanSwapchainContext::CreateDepthResources()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		VkFormat const depthFormat{ deviceContext->FindDepthFormat() };

		m_DepthImage = VulkanImage
		{
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			deviceContext->GetSampleCount(),
			GetExtent().width,
			GetExtent().height,
			1
		};

		m_DepthImage.CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
		// No need to transition since we will do this in the render pass.
	}

	void VulkanSwapchainContext::CreateFrameBuffers(VulkanGraphicsPipeline const* pGraphicsPipeline)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		auto const& imageViews{ GetSwapchainImages() };

		m_SwapChainFrameBuffers.resize(imageViews.size());

		for (size_t i{ 0 }; i < imageViews.size(); ++i)
		{
			std::array<VkImageView, 3> const attachments{ m_ColorImage.imageViews[0], m_DepthImage.imageViews[0],  imageViews[i].imageViews[0] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = pGraphicsPipeline->GetRenderPass();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = GetExtent().width;
			framebufferInfo.height = GetExtent().height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(deviceContext->GetLogicalDevice(), &framebufferInfo, nullptr, &m_SwapChainFrameBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
	}
}
