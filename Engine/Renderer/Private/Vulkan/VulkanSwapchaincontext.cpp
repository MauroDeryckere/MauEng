#include "RendererPCH.h"
#include "VulkanCommandPoolManager.h"
#include "VulkanDescriptorContext.h"

#include "VulkanSwapchainContext.h"
#include "VulkanSurfaceContext.h"
#include "VulkanDeviceContext.h"
#include "VulkanGraphicsPipelineContext.h"
#include "Passes/ClearAttachments.h"

namespace MauRen
{
	void VulkanSwapchainContext::PreInitialize(VulkanSurfaceContext const* pVulkanSurfaceContext)
	{
		m_ColorFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		m_DepthFormat = deviceContext->FindDepthFormat();

		SwapChainSupportDetails const swapChainSupport{ QuerySwapchainSupport(deviceContext->GetPhysicalDevice(), pVulkanSurfaceContext->GetWindowSurface()) };
		auto format = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		m_SwapChainImageFormat = format.format;
	}

	void VulkanSwapchainContext::Initialize(SDL_Window* pWindow, VulkanSurfaceContext const * pVulkanSurfaceContext, VulkanCommandPoolManager& commandPoolManager, VulkanDescriptorContext& descriptorContext)
	{
		CreateSwapchain(pWindow, pVulkanSurfaceContext);
		CreateImageViews();

		CreateColorResources(commandPoolManager, descriptorContext);
		CreateDepthResources(commandPoolManager, descriptorContext);
		CreateGBuffers(commandPoolManager, descriptorContext);
	}

	void VulkanSwapchainContext::ReCreate(SDL_Window* pWindow, VulkanGraphicsPipelineContext const* pGraphicsPipeline, VulkanSurfaceContext const* pVulkanSurfaceContext, VulkanCommandPoolManager& commandPoolManager, VulkanDescriptorContext& descriptorContext)
	{
		Destroy();

		CreateSwapchain(pWindow, pVulkanSurfaceContext);
		CreateImageViews();
		CreateColorResources(commandPoolManager , descriptorContext);
		CreateDepthResources(commandPoolManager, descriptorContext);
		CreateGBuffers(commandPoolManager, descriptorContext);
	}	

	void VulkanSwapchainContext::Destroy()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		for (auto& image : m_DepthImages)
		{
			image.Destroy();
		}
		m_DepthImages.clear();

		for (auto& image : m_ColorImages)
		{
			image.Destroy();
		}
		m_ColorImages.clear();

		// Img destroyed when swapchain is destroyed
		for (auto& image : m_SwapChainImages)
		{
			image.DestroyAllImageViews();
		}
		m_SwapChainImages.clear();

		for (auto& b : m_GBuffers)
		{
			b.Destroy();
		}
		m_GBuffers.clear();

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

		LOGGER.Log(MauCor::ELogPriority::Info, LogRenderer, "Swapchain: {} images, Format: {}, Extent: {} x {} y, Present Mode: {}", imageCount, static_cast<int>(surfaceFormat.format), extent.width, extent.height, static_cast<int>(presentMode));
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

	void VulkanSwapchainContext::CreateColorResources(VulkanCommandPoolManager& commandPoolManager, VulkanDescriptorContext& descriptorContext)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		auto buffer{ commandPoolManager.BeginSingleTimeCommands() };

		for (size_t i{ 0 }; i< MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_ColorImages.emplace_back(
				VulkanImage{
					m_ColorFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					deviceContext->GetSampleCount(),
					GetExtent().width,
					GetExtent().height,
					1,
					1.f
				});
			m_ColorImages.back().CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

			m_ColorImages.back().TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT
			);
		}

		std::vector<VkClearValue> colorClears;
		VkClearValue constexpr clearVal{ .color = {{0, 0, 0, 0}} };
		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			colorClears.emplace_back(clearVal);
		}

		ClearAttachments(buffer, m_ColorImages, colorClears, GetExtent(), {}, {});

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_ColorImages[i].TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				VK_ACCESS_2_SHADER_READ_BIT
			);
		}

		commandPoolManager.EndSingleTimeCommands(buffer);

		std::vector<VkWriteDescriptorSet> descriptorWrites;
		std::vector<VkDescriptorImageInfo> imageInfos;
		descriptorWrites.resize(m_ColorImages.size());
		imageInfos.resize(m_ColorImages.size());

		for (uint32_t i{ 0 }; i < m_ColorImages.size(); ++i)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageView = m_ColorImages[i].imageViews[0];
			imageInfo.imageLayout = m_ColorImages[i].layout;
			imageInfos[i] = imageInfo;

			VkWriteDescriptorSet descriptorWriteColor{};
			descriptorWriteColor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWriteColor.dstSet = descriptorContext.GetDescriptorSets()[i];
			descriptorWriteColor.dstBinding = 10;
			descriptorWriteColor.dstArrayElement = 0;
			descriptorWriteColor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descriptorWriteColor.descriptorCount = 1;
			descriptorWriteColor.pImageInfo = &imageInfos[i];
			descriptorWrites[i] = descriptorWriteColor;
		}

		vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), static_cast<uint32_t>(std::size(descriptorWrites)), descriptorWrites.data(), 0, nullptr);
	}

	void VulkanSwapchainContext::CreateDepthResources(VulkanCommandPoolManager& commandPoolManager, VulkanDescriptorContext& descriptorContext)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		auto buffer{ commandPoolManager.BeginSingleTimeCommands() };

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_DepthImages.emplace_back(VulkanImage
			{
				m_DepthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				deviceContext->GetSampleCount(),
				GetExtent().width,
				GetExtent().height,
				1,
				1.f
			});

			m_DepthImages.back().CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

			m_DepthImages.back().TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
				VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			);
		}


		VkClearValue clearVal{ };
		clearVal.depthStencil.depth = 1.f;
		clearVal.depthStencil.stencil = 0.f;
		for (size_t i{ 0 }; i < m_DepthImages.size(); ++i)
		{
			ClearAttachments(buffer, {}, {}, GetExtent(), m_DepthImages[i], clearVal);
		}

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_DepthImages[i].TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				VK_ACCESS_2_SHADER_READ_BIT
			);
		}

		commandPoolManager.EndSingleTimeCommands(buffer);

		std::vector<VkWriteDescriptorSet> descriptorWrites;
		std::vector<VkDescriptorImageInfo> imageInfos;
		descriptorWrites.resize(m_DepthImages.size());
		imageInfos.resize(m_DepthImages.size());

		for (uint32_t i{ 0 }; i < m_DepthImages.size(); ++i)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageView = m_DepthImages[i].imageViews[0];
			imageInfo.imageLayout = m_DepthImages[i].layout;
			imageInfos[i] = imageInfo;

			VkWriteDescriptorSet descriptorWriteDepth{};
			descriptorWriteDepth.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWriteDepth.dstSet = descriptorContext.GetDescriptorSets()[i];
			descriptorWriteDepth.dstBinding = 9;
			descriptorWriteDepth.dstArrayElement = 0;
			descriptorWriteDepth.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descriptorWriteDepth.descriptorCount = 1;
			descriptorWriteDepth.pImageInfo = &imageInfos[i];
			descriptorWrites[i] = descriptorWriteDepth;
		}

		vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), static_cast<uint32_t>(std::size(descriptorWrites)), descriptorWrites.data(), 0, nullptr);
	}

	void VulkanSwapchainContext::CreateGBuffers(VulkanCommandPoolManager& commandPoolManager, VulkanDescriptorContext& descriptorContext)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };
		VkSampleCountFlagBits const samples{ deviceContext->GetSampleCount() };

		auto buffer{ commandPoolManager.BeginSingleTimeCommands() };

		for (size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			GBuffer g{};
			g.color = VulkanImage
			{
				GBuffer::formats[0],
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT ,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SAMPLE_COUNT_1_BIT,
				GetExtent().width,
				GetExtent().height,
				1,
				1.f
			};
			g.color.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

			g.color.TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT
			);

			g.normal = VulkanImage
			{
				GBuffer::formats[1],
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT ,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SAMPLE_COUNT_1_BIT,
				GetExtent().width,
				GetExtent().height,
				1,
				1.f
			};
			g.normal.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

			g.normal.TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT
			);

			g.metalnessRoughness = VulkanImage
			{
				GBuffer::formats[2],
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT ,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SAMPLE_COUNT_1_BIT,
				GetExtent().width,
				GetExtent().height,
				1,
				1.f
			};
			g.metalnessRoughness.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

			g.metalnessRoughness.TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT
			);

			m_GBuffers.emplace_back(g);
		}

		std::vector<VulkanImage> allColour;
		std::vector<VkClearValue> colorClears;
		VkClearValue constexpr clearVal{ .color = {{0, 0, 0, 0}} };

		for (auto& g : m_GBuffers)
		{
			allColour.clear();
			colorClears.clear();

			allColour.emplace_back(g.color);
			allColour.emplace_back(g.metalnessRoughness);
			allColour.emplace_back(g.normal);
			colorClears.emplace_back(clearVal);
			colorClears.emplace_back(clearVal);
			colorClears.emplace_back(clearVal);

			ClearAttachments(buffer, allColour, colorClears, GetExtent(), {}, {});
		}


		for (auto& g : m_GBuffers)
		{
			g.color.TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				VK_ACCESS_2_SHADER_READ_BIT
			);

			g.normal.TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				VK_ACCESS_2_SHADER_READ_BIT
			);

			g.metalnessRoughness.TransitionImageLayout
			(
				buffer,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
				VK_ACCESS_2_SHADER_READ_BIT
			);
		}

		commandPoolManager.EndSingleTimeCommands(buffer);

		std::vector<VkWriteDescriptorSet> descriptorWrites;
		std::vector<VkDescriptorImageInfo> imageInfos;
		descriptorWrites.resize(m_GBuffers.size() * std::size(GBuffer::formats));
		imageInfos.resize(m_GBuffers.size() * std::size(GBuffer::formats));

		for (uint32_t i{ 0 }; i < m_GBuffers.size(); ++i)
		{
			uint32_t const baseIndex{ i * static_cast<uint32_t>(std::size(GBuffer::formats)) };

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageView = m_GBuffers[i].color.imageViews[0];
				imageInfo.imageLayout = m_GBuffers[i].color.layout;
				imageInfos[baseIndex + 0] = imageInfo;

				VkWriteDescriptorSet descriptorWriteColor = {};
				descriptorWriteColor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWriteColor.dstSet = descriptorContext.GetDescriptorSets()[i];
				descriptorWriteColor.dstBinding = 6;
				descriptorWriteColor.dstArrayElement = 0;
				descriptorWriteColor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				descriptorWriteColor.descriptorCount = 1;
				descriptorWriteColor.pImageInfo = &imageInfos[baseIndex + 0];
				descriptorWrites[baseIndex + 0] = descriptorWriteColor;
			}

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageView = m_GBuffers[i].normal.imageViews[0];
				imageInfo.imageLayout = m_GBuffers[i].normal.layout;
				imageInfos[baseIndex + 1] = imageInfo;

				VkWriteDescriptorSet descriptorWriteNormal = {};
				descriptorWriteNormal.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWriteNormal.dstSet = descriptorContext.GetDescriptorSets()[i];
				descriptorWriteNormal.dstBinding = 7;
				descriptorWriteNormal.dstArrayElement = 0;
				descriptorWriteNormal.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				descriptorWriteNormal.descriptorCount = 1;
				descriptorWriteNormal.pImageInfo = &imageInfos[baseIndex + 1];
				descriptorWrites[baseIndex + 1] = descriptorWriteNormal;
			}

			{
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageView = m_GBuffers[i].metalnessRoughness.imageViews[0];
				imageInfo.imageLayout = m_GBuffers[i].metalnessRoughness.layout;
				imageInfos[baseIndex + 2] = imageInfo;

				VkWriteDescriptorSet descriptorWriteMetal = {};
				descriptorWriteMetal.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWriteMetal.dstSet = descriptorContext.GetDescriptorSets()[i];
				descriptorWriteMetal.dstBinding = 8;
				descriptorWriteMetal.dstArrayElement = 0;
				descriptorWriteMetal.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				descriptorWriteMetal.descriptorCount = 1;
				descriptorWriteMetal.pImageInfo = &imageInfos[baseIndex + 2];
				descriptorWrites[baseIndex + 2] = descriptorWriteMetal;
			}
		}

		vkUpdateDescriptorSets(deviceContext->GetLogicalDevice(), static_cast<uint32_t>(std::size(descriptorWrites)), descriptorWrites.data(), 0, nullptr);
	}
}
