#include "VulkanInstanceContext.h"

#include "VulkanDebugContext.h"

namespace MauRen
{
	void VulkanInstanceContext::Initialize()
	{
		CreateVulkanInstance();
	}

	void VulkanInstanceContext::Destroy()
	{
		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanInstanceContext::CreateVulkanInstance()
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

			VulkanDebugContext::PopulateDebugMessengerCreateInfo(debugCreateInfo);
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


		if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
	}

	bool VulkanInstanceContext::CheckValidationLayerSupport()
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

	std::vector<char const*> VulkanInstanceContext::GetRequiredExtensions()
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

	bool VulkanInstanceContext::CheckInstanceExtensionsSupport(uint32_t extensionCount, std::vector<char const*> const& extensions, std::vector<VkExtensionProperties> const& availableExtensions)
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
}
