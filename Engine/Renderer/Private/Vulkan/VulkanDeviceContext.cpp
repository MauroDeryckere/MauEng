#include "VulkanDeviceContext.h"

#include "VulkanSwapchainContext.h"

namespace MauRen
{
	VulkanDeviceContext::VulkanDeviceContext(VulkanSurfaceContext* pVulkanSurfaceContext, VulkanInstanceContext* pVulkanInstanceContext) :
		m_pVulkanSurfaceContext{ pVulkanSurfaceContext },
		m_InstanceContext{ pVulkanInstanceContext }

	{
		SelectPhysicalDevice();
		CreateLogicalDevice();
	}
	
	VulkanDeviceContext::~VulkanDeviceContext()
	{
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	QueueFamilyIndices VulkanDeviceContext::FindQueueFamilies() const noexcept
	{
		return FindQueueFamilies(m_PhysicalDevice);
	}

	VkFormat VulkanDeviceContext::FindDepthFormat() const noexcept
	{
		return FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool VulkanDeviceContext::HasStencilComponent(VkFormat format) noexcept
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	VkFormat VulkanDeviceContext::FindSupportedFormat(std::vector<VkFormat> const& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
	{
		for (VkFormat const& format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
			{
				return format;
			}

			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
			{
				return format;
			}
		}

		throw std::runtime_error("Failed to find supported format!");
	}

	VkSampleCountFlagBits VulkanDeviceContext::GetMaxUsableSampleCount() const noexcept
	{
		return VK_SAMPLE_COUNT_1_BIT;

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags const counts{ physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts };
		if (counts & VK_SAMPLE_COUNT_64_BIT)
		{
			return VK_SAMPLE_COUNT_64_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_32_BIT)
		{
			return VK_SAMPLE_COUNT_32_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_16_BIT)
		{
			return VK_SAMPLE_COUNT_16_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_8_BIT)
		{
			return VK_SAMPLE_COUNT_8_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_4_BIT)
		{
			return VK_SAMPLE_COUNT_4_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_2_BIT)
		{
			return VK_SAMPLE_COUNT_2_BIT;
		}

		return VK_SAMPLE_COUNT_1_BIT;
	}

	void VulkanDeviceContext::SelectPhysicalDevice()
	{
		uint32_t deviceCount{ 0 };
		vkEnumeratePhysicalDevices(m_InstanceContext->GetInstance(), &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_InstanceContext->GetInstance(), &deviceCount, devices.data());

		LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Selecting physical device.");

		// Automatically select best option
		uint32_t bestScore{ 0 };
		if constexpr (AUTO_SELECT_PHYSICAL_DEVICE)
		{
			std::string deviceList{};

			for (auto const& device : devices)
			{
				if (IsPhysicalDeviceSuitable(device))
				{
					VkPhysicalDeviceProperties props;
					vkGetPhysicalDeviceProperties(device, &props);

					deviceList += fmt::format("\t{}\n", props.deviceName);


					auto const score{ RateDeviceSuitability( device) };
					if (score > bestScore)
					{
						bestScore = score;
						m_PhysicalDevice = device;
						m_MsaaSamples = GetMaxUsableSampleCount();
					}
				}
			}

			LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Available Vulkan physical devices: \n {}", deviceList);


			if (m_PhysicalDevice == VK_NULL_HANDLE)
			{
				throw std::runtime_error("failed to find a suitable GPU!");
			}

			VkPhysicalDeviceProperties selectedProps;
			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &selectedProps);

			LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Selected GPU: {}  (score: {} )" , selectedProps.deviceName, bestScore);

			// for bindless
			uint32_t const maxSampledImages{ selectedProps.limits.maxPerStageDescriptorSampledImages };
			const_cast<uint32_t&>(MAX_SAMPLED_IMAGES) = maxSampledImages;
			LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Max per stage sampled images: {}", maxSampledImages);

			uint32_t const maxDescriptorsPerStage{ selectedProps.limits.maxPerStageResources };
			const_cast<uint32_t&>(MAX_DESCRIPTORS_STAGE) = maxDescriptorsPerStage;
			LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Max per stage descriptors: {}", maxDescriptorsPerStage);

			uint32_t const maxDescriptorsPerSet{ selectedProps.limits.maxDescriptorSetSampledImages };
			const_cast<uint32_t&>(MAX_DESCRIPTORS_SET) = maxDescriptorsPerSet;
		}

		// Allow user to manually select 
		else
		{
			// Pair<Device, score>
			std::vector<std::pair<VkPhysicalDevice, uint32_t>> selectableDevices{};

			std::string deviceList{};

			for (auto const& device : devices)
			{
				if (IsPhysicalDeviceSuitable(device))
				{
					auto const score{ RateDeviceSuitability(device) };
					selectableDevices.emplace_back(device, score);

					VkPhysicalDeviceProperties props;
					vkGetPhysicalDeviceProperties(device, &props);

					deviceList += fmt::format("\t{}\t{} score: {} \n", selectableDevices.size() - 1, props.deviceName, score);
				}
			}

			LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Available Vulkan physical devices: \n {}", deviceList);


			if (selectableDevices.empty())
			{
				throw std::runtime_error("failed to find a suitable GPU!");
			}

			int selectedIndex = -1;

			LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Enter the index of the GPU to use :");
			std::cin >> selectedIndex;

			selectedIndex = std::clamp(selectedIndex, 0, static_cast<int>(selectableDevices.size()));

			m_PhysicalDevice = selectableDevices[selectedIndex].first;
			VkPhysicalDeviceProperties selectedProps;
			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &selectedProps);
			LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Selected GPU: {}", selectedProps.deviceName);

			// for bindless
			uint32_t const maxSampledImages{ selectedProps.limits.maxPerStageDescriptorSampledImages };
			const_cast<uint32_t&>(MAX_SAMPLED_IMAGES) = maxSampledImages;
			LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Max per stage sampled images: {}", maxSampledImages);

			uint32_t const maxDescriptorsPerStage{ selectedProps.limits.maxPerStageResources };
			const_cast<uint32_t&>(MAX_DESCRIPTORS_STAGE) = maxDescriptorsPerStage;
			LOGGER.Log(MauCor::LogPriority::Info, MauCor::LogCategory::Renderer, "Max per stage descriptors: {}", maxDescriptorsPerStage);

			uint32_t const maxDescriptorsPerSet{ selectedProps.limits.maxDescriptorSetSampledImages };
			const_cast<uint32_t&>(MAX_DESCRIPTORS_SET) = maxDescriptorsPerSet;
		}
	}

	void VulkanDeviceContext::CreateLogicalDevice()
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

		// Enable all required device features
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		deviceFeatures.multiDrawIndirect = VK_TRUE;
		
		VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexingFeatures.runtimeDescriptorArray = VK_TRUE;
		indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
		indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		indexingFeatures.pNext = nullptr;

		VkPhysicalDeviceVulkan13Features features13
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &indexingFeatures
		};
		features13.synchronization2 = VK_TRUE;
		features13.dynamicRendering = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // Since it's a set, only the unique amt of families are added.

		createInfo.pNext = &features13;
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
				LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Using unified graphics & present queue");
				m_IsUsingUnifiedGraphicsPresentQueue = true;

				vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_UnifiedGraphicsPresentQueue);
			}
			else
			{
				LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Using separate graphics & present queue");
				m_IsUsingUnifiedGraphicsPresentQueue = false;

				vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
				vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_PresentQueue);
			}
		}
		else
		{
			LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Using separate graphics & present queue");
			m_IsUsingUnifiedGraphicsPresentQueue = false;

			vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
			vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_PresentQueue);
		}
	}

	bool VulkanDeviceContext::CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		return std::ranges::all_of(DEVICE_EXTENSIONS, [&](std::string_view ext)
			{
				if (std::ranges::any_of(availableExtensions, [ext](const VkExtensionProperties& availableExt) { return ext == availableExt.extensionName; }))
				{
					LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Required extension \" {} \" is supported", ext);
					return true;
				}
				LOGGER.Log(MauCor::LogPriority::Fatal, MauCor::LogCategory::Renderer, "Required extension \" {} \" is NOT supported", ext);
				return false;
			});
	}

	uint32_t VulkanDeviceContext::RateDeviceSuitability(VkPhysicalDevice device) const
	{
		QueueFamilyIndices const indices{ FindQueueFamilies(device) };

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		uint32_t score{ 0 };
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		score += deviceProperties.limits.maxImageDimension2D;

		// Prefer devices with unified graphics & present queue family
		if (indices.IsGraphicsPresentUnified())
		{
			score += 500;
		}

		return score;
	}

	bool VulkanDeviceContext::IsPhysicalDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices const indices{ FindQueueFamilies(device) };

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		VkPhysicalDeviceVulkan13Features vulkan13Features{};
		vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

		VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		vulkan13Features.pNext = &indexingFeatures;

		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.pNext = &vulkan13Features;

		vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);

		bool const extensionsSupported{ CheckPhysicalDeviceExtensionSupport(device) };

		bool swapChainAdequate{ false };
		if (extensionsSupported)
		{
			SwapChainSupportDetails const swapChainSupport{ VulkanSwapchainContext::QuerySwapchainSupport(device, m_pVulkanSurfaceContext->GetWindowSurface()) };
			swapChainAdequate = not swapChainSupport.formats.empty()
							&& not swapChainSupport.presentModes.empty()
							&& deviceFeatures.samplerAnisotropy	// Could also not enforce and set a bool that's reused here 
							&& deviceFeatures.fillModeNonSolid

							&& indexingFeatures.runtimeDescriptorArray
							&& indexingFeatures.descriptorBindingPartiallyBound
							&& indexingFeatures.descriptorBindingVariableDescriptorCount
							&& indexingFeatures.descriptorBindingVariableDescriptorCount

							&& vulkan13Features.dynamicRendering
							&& vulkan13Features.synchronization2

							&& deviceFeatures.multiDrawIndirect;
		}

		return indices.IsComplete() and extensionsSupported and swapChainAdequate;
	}

	QueueFamilyIndices VulkanDeviceContext::FindQueueFamilies(VkPhysicalDevice device) const noexcept
	{
		QueueFamilyIndices indices{};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());


		for (uint32_t i{ 0 }; i < static_cast<uint32_t>(queueFamilies.size()); ++i)
		{
			VkBool32 presentSupport{ false };
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_pVulkanSurfaceContext->GetWindowSurface(), &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			// Temporary
			if (indices.IsComplete())
			{
				break;
			}
		}
		return indices;
	}
}
