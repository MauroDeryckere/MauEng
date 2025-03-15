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
#include <span>

GLFWwindow* g_Window{ nullptr };
VkInstance g_Instance;

VkDebugUtilsMessengerEXT g_DebugMessenger;

#ifdef NDEBUG
	bool constexpr ENABLE_VULKAN_VALIDATION_LAYERS{ false };
#else
	bool constexpr ENABLE_VULKAN_VALIDATION_LAYERS{ true  };
#endif

std::vector const VULKAN_VALIDATION_LAYERS{ "VK_LAYER_KHRONOS_validation" };


void InitWindow()
{
	glfwInit();
 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	uint32_t constexpr WIDTH{ 800 };
	uint32_t constexpr HEIGHT{ 600 };

	g_Window = glfwCreateWindow(WIDTH, HEIGHT, "Mauro Deryckere - Vulkan Project", nullptr, nullptr);

}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

// Checks if all requested validation layers are available
bool CheckvalidationLayerSupport()
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

std::vector<char const*> GetRequiredExtensions()
{
	// Need an extension to interface with the window system
	// GLFW returns the extensions it needs to that -> pass to struct
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
bool CheckExtensionsSupport(uint32_t extensionCount, std::vector<char const*> const& extensions, std::vector<VkExtensionProperties> const& availableExtensions)
{
	std::cout << "\n";

	return std::ranges::all_of(extensions, [&](std::string_view ext)
			{
				if (std::ranges::any_of(availableExtensions, [ext](const VkExtensionProperties& availableExt){ return ext == availableExt.extensionName; }))
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
		if (CheckvalidationLayerSupport())
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

	if (!CheckExtensionsSupport(static_cast<uint32_t>(extensions.size()), extensions, availableExtensions))
	{
		throw std::runtime_error("Not all required extensions are supported!");
	}
	std::cout << "All required extensions are supported.\n";



	if (vkCreateInstance(&createInfo, nullptr, &g_Instance) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create instance!");
	}
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const* pCreateInfo, VkAllocationCallbacks const* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
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

void InitVulkan()
{
	CreateVulkanInstance();
	SetupDebugMessenger();
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
	if constexpr (ENABLE_VULKAN_VALIDATION_LAYERS) 
	{
		DestroyDebugUtilsMessengerEXT(g_Instance, g_DebugMessenger, nullptr);
	}

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

int main()
{
	try 
	{
		Run();
	}
	catch (const std::exception& e) 
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
