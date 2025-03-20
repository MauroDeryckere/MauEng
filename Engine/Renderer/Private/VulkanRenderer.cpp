#include "VulkanRenderer.h"

namespace MauRen
{
	VulkanRenderer::VulkanRenderer(GLFWwindow* pWindow) :
		Renderer{ pWindow },
		m_InstanceContext{},
		m_SurfaceContext{ m_InstanceContext, pWindow },
		m_DebugContext{ m_InstanceContext }

	{

	}

	VulkanRenderer::~VulkanRenderer()
	{
		Cleanup();
	}

	void VulkanRenderer::Initialize(GLFWwindow* pWindow)
	{
		m_pWindow = pWindow;
		InitVulkan();
	}

	void VulkanRenderer::Render()
	{
		//TODO
	}

	void VulkanRenderer::InitVulkan()
	{
	//	CreateVulkanInstance();
	//	SetupDebugMessenger();
	//	CreateWindowSurface();
		SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
	}

	void VulkanRenderer::SelectPhysicalDevice()
	{
		uint32_t deviceCount{ 0 };
		vkEnumeratePhysicalDevices(m_InstanceContext.GetInstance(), &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_InstanceContext.GetInstance(), &deviceCount, devices.data());

		std::cout << "\nSelecting physical device.\n";

		// Automatically select best option
		uint32_t bestScore{ 0 };
		if constexpr (AUTO_SELECT_PHYSICAL_DEVICE)
		{
			std::cout << "Available Vulkan physical devices:\n";

			for (auto const& device : devices)
			{
				if (IsPhysicalDeviceSuitable(device))
				{
					VkPhysicalDeviceProperties props;
					vkGetPhysicalDeviceProperties(device, &props);

					std::cout << "\t" << props.deviceName << "\n";


					auto const score{ RateDeviceSuitability(device) };
					if (score > bestScore)
					{
						bestScore = score;
						m_PhysicalDevice = device;
					}
				}
			}

			if (m_PhysicalDevice == VK_NULL_HANDLE)
			{
				throw std::runtime_error("failed to find a suitable GPU!");
			}

			VkPhysicalDeviceProperties selectedProps;
			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &selectedProps);
			std::cout << "\nSelected GPU: " << selectedProps.deviceName << " (score: " << bestScore << ")\n";
		}

		// Allow user to manually select 
		else
		{
			// Pair<Device, score>
			std::vector<std::pair<VkPhysicalDevice, uint32_t>> selectableDevices{};
			std::cout << "Available Vulkan physical devices:\n";

			for (auto const& device : devices)
			{
				if (IsPhysicalDeviceSuitable(device))
				{
					auto const score{ RateDeviceSuitability(device) };
					selectableDevices.emplace_back(device, score);

					VkPhysicalDeviceProperties props;
					vkGetPhysicalDeviceProperties(device, &props);

					std::cout << selectableDevices.size() - 1 << "\t" << props.deviceName << " score: " << score << ")\n";
				}
			}

			if (selectableDevices.empty())
			{
				throw std::runtime_error("failed to find a suitable GPU!");
			}

			int selectedIndex = -1;
			std::cout << "\nEnter the index of the GPU to use: ";
			std::cin >> selectedIndex;

			selectedIndex = std::clamp(selectedIndex, 0, static_cast<int>(selectableDevices.size()));

			m_PhysicalDevice = selectableDevices[selectedIndex].first;
			VkPhysicalDeviceProperties selectedProps;
			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &selectedProps);
			std::cout << "Selected GPU: " << selectedProps.deviceName << "\n";
		}
	}

	void VulkanRenderer::CreateLogicalDevice()
	{
		QueueFamilyIndices const indices{ FindQueueFamilies(m_PhysicalDevice) };
		assert(indices.IsComplete());

		std::set<uint32_t> uniqueQueueFamilies;

		uniqueQueueFamilies.insert(indices.graphicsFamily.value());
		uniqueQueueFamilies.insert(indices.presentFamily.value());

		float constexpr queuePriority{ 1.0f };

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		for (auto const queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.emplace_back(queueCreateInfo);
		}

		// TODO Enable Vulkan device features -not necessary currenrly 
		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // Since it's a set, only the unique amt of families are added.

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
		createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

		if constexpr (ENABLE_VULKAN_VALIDATION_LAYERS)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(VULKAN_VALIDATION_LAYERS.size());
			createInfo.ppEnabledLayerNames = VULKAN_VALIDATION_LAYERS.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device!");
		}

		if constexpr (not FORCE_SEPARATE_GRAPHICS_PRESENT_QUEUES)
		{
			// If the families are the same (unified), we can just use one queue for both
			if (indices.IsGraphicsPresentUnified())
			{
				std::cout << "\nLog: Using unified graphics & present queue \n";

				m_IsUsingUnifiedGraphicsPresentQueue = true;

				vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_UnifiedGraphicsPresentQueue);
			}
			else
			{
				std::cout << "\nLog: Using separate graphics & present queue \n";

				vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
				vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_PresentQueue);
			}
		}
		else
		{
			std::cout << "\nLog: Using separate graphics & present queue \n";

			vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
			vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_PresentQueue);
		}
	}

	void VulkanRenderer::CreateSwapChain()
	{
		SwapChainSupportDetails const swapChainSupport{ QuerySwapChainSupport(m_PhysicalDevice) };

		VkSurfaceFormatKHR const surfaceFormat{ ChooseSwapSurfaceFormat(swapChainSupport.formats) };
		VkPresentModeKHR const presentMode{ ChooseSwapPresentMode(swapChainSupport.presentModes) };
		VkExtent2D const extent{ ChooseSwapExtent(swapChainSupport.capabilities) };

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
		createInfo.surface = m_SurfaceContext.GetWindowSurface();

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
		uint32_t queueFamilyIndices[]{ indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (m_IsUsingUnifiedGraphicsPresentQueue)
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

		if (vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;

		std::cout << "Swapchain: " << imageCount << " images, Format: " << surfaceFormat.format
			<< ", Extent: " << extent.width << "x" << extent.height
			<< ", Present Mode: " << presentMode << std::endl;
	}

	void VulkanRenderer::CreateImageViews()
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
			if (vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image views!");
			}
		}
	}


	void VulkanRenderer::Cleanup()
	{
		for (auto const& imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_LogicalDevice, imageView, nullptr);
		}

		vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);

		vkDestroyDevice(m_LogicalDevice, nullptr);


	}
}


