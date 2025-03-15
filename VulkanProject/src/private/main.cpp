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

// Checks if all requested extensions are available
bool CheckExtensionsSupport(uint32_t glfwExtensionCount, char const** glfwExtensions, std::vector<VkExtensionProperties> const& availableExtensions)
{
	std::span requiredExtensions(glfwExtensions, glfwExtensionCount);

	std::cout << "\n";

	return std::ranges::all_of(requiredExtensions, [&](std::string_view ext) 
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

void CreateVulkanInstance()
{
	if (ENABLE_VULKAN_VALIDATION_LAYERS)
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

	for (auto const& extension : availableExtensions)
	{
		std::cout << '\t' << extension.extensionName << '\n';
	}
	// -------------------------------------

	// Need an extension to interface with the window system
	// GLFW returns the extensions it needs to that -> pass to struct
	uint32_t glfwExtensionCount{ 0 };
	char const** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

	// Which global extensions and validation layers do we want to use
	// ! For entire program, not a specific device
	VkInstanceCreateInfo createInfo{ };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	// Setup validation layers (in debug mode)
	if constexpr (ENABLE_VULKAN_VALIDATION_LAYERS) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(VULKAN_VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VULKAN_VALIDATION_LAYERS.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

	if (!CheckExtensionsSupport(glfwExtensionCount, glfwExtensions, availableExtensions))
	{
		throw std::runtime_error("Not all required GLFW extensions are supported!");
	}
	std::cout << "All required GLFW extensions are supported.\n";


	if (vkCreateInstance(&createInfo, nullptr, &g_Instance) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create instance!");
	}
}


void InitVulkan()
{
	CreateVulkanInstance();

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
