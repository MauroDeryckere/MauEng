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


					auto const score{ RateDeviceSuitability( device) };
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
				m_IsUsingUnifiedGraphicsPresentQueue = false;

				vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
				vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_PresentQueue);
			}
		}
		else
		{
			std::cout << "\nLog: Using separate graphics & present queue \n";
			m_IsUsingUnifiedGraphicsPresentQueue = false;

			vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
			vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_PresentQueue);
		}
	}

	bool VulkanDeviceContext::CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
	{
		std::cout << "\n";

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		return std::ranges::all_of(DEVICE_EXTENSIONS, [&](std::string_view ext)
			{
				if (std::ranges::any_of(availableExtensions, [ext](const VkExtensionProperties& availableExt) { return ext == availableExt.extensionName; }))
				{
					std::cout << "Required extension \"" << ext << "\" is supported.\n";
					return true;
				}

				std::cerr << "Required extension \"" << ext << "\" is not supported.\n";
				return false;
			});
	}

	uint32_t VulkanDeviceContext::RateDeviceSuitability(VkPhysicalDevice device)
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

		bool const extensionsSupported{ CheckPhysicalDeviceExtensionSupport(device) };

		bool swapChainAdequate{ false };
		if (extensionsSupported)
		{
			SwapChainSupportDetails const swapChainSupport{ VulkanSwapchainContext::QuerySwapchainSupport(device, m_pVulkanSurfaceContext->GetWindowSurface()) };
			swapChainAdequate = not swapChainSupport.formats.empty() && not swapChainSupport.presentModes.empty();
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
