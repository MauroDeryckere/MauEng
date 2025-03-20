#include "Renderer.h"

/* Sources
 * https://vulkan-tutorial.com/Introduction
 */

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <algorithm>

#include <optional>

#include <set>

 // globals, to be moved to the renderer in the future

GLFWwindow* g_Window{ nullptr };
VkInstance g_Instance;
// This object will be implicitly destroyed when the VkInstance is destroyed, so we won't need to do anything new in the cleanup function.
VkPhysicalDevice g_PhysicalDevice{ VK_NULL_HANDLE };

VkDevice g_LogicalDevice;

// Device queues are implicitly cleaned up when the device is destroyed, so we don't need to do anything in cleanup.
VkQueue g_GraphicsQueue;
VkQueue g_PresentQueue;

// Using a unified queue may result in less overhead and a performance boost
bool constexpr FORCE_SEPARATE_GRAPHICS_PRESENT_QUEUES{ false };

bool g_IsUsingUnifiedGraphicsPresentQueue{ false };
VkQueue g_UnifiedGraphicsPresentQueue;


VkDebugUtilsMessengerEXT g_DebugMessenger;

VkSurfaceKHR g_WindowSurface;

VkSwapchainKHR g_SwapChain;

std::vector<VkImage> g_SwapChainImages;
VkFormat g_SwapChainImageFormat;
VkExtent2D g_SwapChainExtent;

std::vector<VkImageView> g_SwapChainImageViews;

#ifdef NDEBUG
bool constexpr ENABLE_VULKAN_VALIDATION_LAYERS{ false };
#else
bool constexpr ENABLE_VULKAN_VALIDATION_LAYERS{ true };
#endif

std::vector const VULKAN_VALIDATION_LAYERS
{
	"VK_LAYER_KHRONOS_validation",
};

std::vector const DEVICE_EXTENSIONS
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool constexpr AUTO_SELECT_PHYSICAL_DEVICE{ true };

struct QueueFamilyIndices final
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	[[nodiscard]] bool IsComplete() const noexcept
	{
		return graphicsFamily.has_value() and presentFamily.has_value();
	}

	[[nodiscard]] bool IsGraphicsPresentUnified() const noexcept
	{
		return (graphicsFamily.has_value() and presentFamily.has_value())
			and graphicsFamily.value() == presentFamily.value();
	}
};

[[nodiscard]] QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices{};

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());


	for (uint32_t i{ 0 }; i < static_cast<uint32_t>(queueFamilies.size()); ++i)
	{
		VkBool32 presentSupport{ false };
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, g_WindowSurface, &presentSupport);

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


void InitWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	uint32_t constexpr WIDTH{ 800 };
	uint32_t constexpr HEIGHT{ 600 };

	g_Window = glfwCreateWindow(WIDTH, HEIGHT, "Mauro Deryckere - Vulkan Project", nullptr, nullptr);

}

[[nodiscard]] static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

// Checks if all requested validation layers are available
[[nodiscard]] bool CheckValidationLayerSupport()
{
	uint32_t layerCount{ 0 };
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	std::cout << "\n";

	return std::ranges::all_of(VULKAN_VALIDATION_LAYERS, [&](std::string_view layerName)
		{
			if (std::ranges::any_of(availableLayers, [&](const VkLayerProperties& props) { return layerName == props.layerName; }))
			{
				std::cout << "Required validation layer \"" << layerName << "\" is supported.\n";
				return true;
			}

			std::cerr << "Required validation layer \"" << layerName << "\" is not supported.\n";
			return false;
		});
}

[[nodiscard]] std::vector<char const*> GetRequiredExtensions()
{
	// Need an extension to interface with the window system
	// GLFW returns the extensions it needs to do that -> pass to struct
	uint32_t glfwExtensionCount{ 0 };
	char const** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

	std::vector<char const*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if constexpr (ENABLE_VULKAN_VALIDATION_LAYERS)
	{
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

// Checks if all requested extensions are available
[[nodiscard]] bool CheckInstanceExtensionsSupport(uint32_t extensionCount, std::vector<char const*> const& extensions, std::vector<VkExtensionProperties> const& availableExtensions)
{
	std::cout << "\n";

	return std::ranges::all_of(extensions, [&](std::string_view ext)
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

// Checks if all requested extensions are available
[[nodiscard]] bool CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
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

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	// "I've specified all types except for VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT here to receive notifications about possible problems while leaving out verbose general debug info."
	// https://docs.vulkan.org/spec/latest/chapters/debugging.html#VK_EXT_debug_utils

	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr; // Optional
}


void CreateVulkanInstance()
{
	if constexpr (ENABLE_VULKAN_VALIDATION_LAYERS)
	{
		if (CheckValidationLayerSupport())
		{
			std::cout << "All required validation layers are supported.\n";
		}
		else
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Mauro Deryckere - Vulkan Project";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Provide details about Vulkan support
	uint32_t extensionCount{ 0 };
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	std::cout << "available extensions:\n";

	for (const auto& [extensionName, specVersion] : availableExtensions)
	{
		std::cout << "\tExtension: " << extensionName << ", Version: " << specVersion << '\n';
	}
	// -------------------------------------


	// Which global extensions and validation layers do we want to use
	// ! For entire program, not a specific device
	VkInstanceCreateInfo createInfo{ };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto const& extensions{ GetRequiredExtensions() };
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Setup validation layers (in debug mode)
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if constexpr (ENABLE_VULKAN_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(VULKAN_VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VULKAN_VALIDATION_LAYERS.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (!CheckInstanceExtensionsSupport(static_cast<uint32_t>(extensions.size()), extensions, availableExtensions))
	{
		throw std::runtime_error("Not all required extensions are supported!");
	}
	std::cout << "All required extensions are supported.\n";



	if (vkCreateInstance(&createInfo, nullptr, &g_Instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

[[nodiscard]] VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const* pCreateInfo, VkAllocationCallbacks const* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	if (auto func{ (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT") })
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks const* pAllocator)
{
	// ! Make sure that this function is either a static class function or a function outside the class.

	if (auto func{ (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT") })
	{
		func(instance, debugMessenger, pAllocator);
	}
}

void SetupDebugMessenger()
{
	if constexpr (!ENABLE_VULKAN_VALIDATION_LAYERS) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(g_Instance, &createInfo, nullptr, &g_DebugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}

}

struct SwapChainSupportDetails final
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	// basic surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, g_WindowSurface, &details.capabilities);

	// querying the supported surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_WindowSurface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_WindowSurface, &formatCount, details.formats.data());
	}

	// querying the supported presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_WindowSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_WindowSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

// Is a physical device suitable for our application
[[nodiscard]] bool IsPhysicalDeviceSuitable(VkPhysicalDevice device)
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
		SwapChainSupportDetails const swapChainSupport{ QuerySwapChainSupport(device) };
		swapChainAdequate = not swapChainSupport.formats.empty() && not swapChainSupport.presentModes.empty();
	}

	return indices.IsComplete() and extensionsSupported and swapChainAdequate;
}

// Give a physical device a rating to allow automatically selecting the "best" option
[[nodiscard]] uint32_t RateDeviceSuitability(VkPhysicalDevice device)
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


// Select which physical device vulkan will use
void SelectPhysicalDevice()
{
	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(g_Instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(g_Instance, &deviceCount, devices.data());

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
					g_PhysicalDevice = device;
				}
			}
		}

		if (g_PhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}

		VkPhysicalDeviceProperties selectedProps;
		vkGetPhysicalDeviceProperties(g_PhysicalDevice, &selectedProps);
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

		g_PhysicalDevice = selectableDevices[selectedIndex].first;
		VkPhysicalDeviceProperties selectedProps;
		vkGetPhysicalDeviceProperties(g_PhysicalDevice, &selectedProps);
		std::cout << "Selected GPU: " << selectedProps.deviceName << "\n";
	}
}

void CreateLogicalDevice()
{
	QueueFamilyIndices const indices{ FindQueueFamilies(g_PhysicalDevice) };
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

	if (vkCreateDevice(g_PhysicalDevice, &createInfo, nullptr, &g_LogicalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	if constexpr (not FORCE_SEPARATE_GRAPHICS_PRESENT_QUEUES)
	{
		// If the families are the same (unified), we can just use one queue for both
		if (indices.IsGraphicsPresentUnified())
		{
			std::cout << "\nLog: Using unified graphics & present queue \n";

			g_IsUsingUnifiedGraphicsPresentQueue = true;

			vkGetDeviceQueue(g_LogicalDevice, indices.graphicsFamily.value(), 0, &g_UnifiedGraphicsPresentQueue);
		}
		else
		{
			std::cout << "\nLog: Using separate graphics & present queue \n";

			vkGetDeviceQueue(g_LogicalDevice, indices.graphicsFamily.value(), 0, &g_GraphicsQueue);
			vkGetDeviceQueue(g_LogicalDevice, indices.presentFamily.value(), 0, &g_PresentQueue);
		}
	}
	else
	{
		std::cout << "\nLog: Using separate graphics & present queue \n";

		vkGetDeviceQueue(g_LogicalDevice, indices.graphicsFamily.value(), 0, &g_GraphicsQueue);
		vkGetDeviceQueue(g_LogicalDevice, indices.presentFamily.value(), 0, &g_PresentQueue);
	}
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats)
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

VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes)
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

VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR const& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	int width, height;
	glfwGetFramebufferSize(g_Window, &width, &height);

	VkExtent2D actualExtent
	{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}


void CreateSwapChain()
{
	SwapChainSupportDetails const swapChainSupport{ QuerySwapChainSupport(g_PhysicalDevice) };

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
	createInfo.surface = g_WindowSurface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(g_PhysicalDevice);
	uint32_t queueFamilyIndices[]{ indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (g_IsUsingUnifiedGraphicsPresentQueue)
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

	if (vkCreateSwapchainKHR(g_LogicalDevice, &createInfo, nullptr, &g_SwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(g_LogicalDevice, g_SwapChain, &imageCount, nullptr);
	g_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(g_LogicalDevice, g_SwapChain, &imageCount, g_SwapChainImages.data());

	g_SwapChainImageFormat = surfaceFormat.format;
	g_SwapChainExtent = extent;

	std::cout << "Swapchain: " << imageCount << " images, Format: " << surfaceFormat.format
		<< ", Extent: " << extent.width << "x" << extent.height
		<< ", Present Mode: " << presentMode << std::endl;
}

void CreateWindowSurface()
{
	if (glfwCreateWindowSurface(g_Instance, g_Window, nullptr, &g_WindowSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

void CreateImageViews()
{
	g_SwapChainImageViews.resize(g_SwapChainImages.size());
	for (size_t i = 0; i < g_SwapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = g_SwapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = g_SwapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(g_LogicalDevice, &createInfo, nullptr, &g_SwapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void InitVulkan()
{
	CreateVulkanInstance();
	SetupDebugMessenger();
	CreateWindowSurface();
	SelectPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
}

void MainLoop()
{
	assert(g_Window);
	while (!glfwWindowShouldClose(g_Window))
	{
		glfwPollEvents();
	}
}

void Cleanup()
{
	for (auto const& imageView : g_SwapChainImageViews)
	{
		vkDestroyImageView(g_LogicalDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(g_LogicalDevice, g_SwapChain, nullptr);

	vkDestroyDevice(g_LogicalDevice, nullptr);

	if constexpr (ENABLE_VULKAN_VALIDATION_LAYERS)
	{
		DestroyDebugUtilsMessengerEXT(g_Instance, g_DebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(g_Instance, g_WindowSurface, nullptr);

	vkDestroyInstance(g_Instance, nullptr);

	glfwDestroyWindow(g_Window);
	glfwTerminate();
}

void Run()
{
	InitWindow();
	InitVulkan();
	MainLoop();
	Cleanup();
}

void Renderer::Renderer::RenderRun()
{
	try
	{
		Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	//	return EXIT_FAILURE;
	}

//	return EXIT_SUCCESS;
}
