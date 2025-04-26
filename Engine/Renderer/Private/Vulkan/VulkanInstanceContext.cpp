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
				LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "All required validation layers are supported.");
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
		appInfo.apiVersion = VK_API_VERSION_1_3;

		// Provide details about Vulkan support
		uint32_t extensionCount{ 0 };
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Available extensions: ");

		for (const auto& [extensionName, specVersion] : availableExtensions)
		{
			LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "\tExtension: {}, versions: {} ", extensionName, specVersion);
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

		LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "All required extensions are supported.");

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

		return std::ranges::all_of(VULKAN_VALIDATION_LAYERS, [&](std::string_view layerName)
			{
				if (std::ranges::any_of(availableLayers, [&](const VkLayerProperties& props) { return layerName == props.layerName; }))
				{
					LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Required validation layer \" {} \" is supported.", layerName);
					return true;
				}

				LOGGER.Log(MauCor::LogPriority::Fatal, MauCor::LogCategory::Renderer, "Required validation layer \" {} \" is NOT supported.", layerName);
				return false;
			});
	}

	std::vector<char const*> VulkanInstanceContext::GetRequiredExtensions()
	{
		// Need an extension to interface with the window system
		// GLFW returns the extensions it needs to do that -> pass to struct
		uint32_t extCount{ 0 };
		auto sdlExtensions { SDL_Vulkan_GetInstanceExtensions(&extCount) };
//		 glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };extCount

		std::vector<char const*> extensions(sdlExtensions, sdlExtensions + extCount);

		if constexpr (ENABLE_VULKAN_VALIDATION_LAYERS)
		{
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	bool VulkanInstanceContext::CheckInstanceExtensionsSupport(uint32_t extensionCount, std::vector<char const*> const& extensions, std::vector<VkExtensionProperties> const& availableExtensions)
	{
		return std::ranges::all_of(extensions, [&](std::string_view ext)
			{
				if (std::ranges::any_of(availableExtensions, [ext](const VkExtensionProperties& availableExt) { return ext == availableExt.extensionName; }))
				{
					LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Required extension \" {} \" is supported.", ext);
					return true;
				}

				LOGGER.Log(MauCor::LogPriority::Fatal, MauCor::LogCategory::Renderer, "Required extension \" {} \" is NOT supported.", ext);
				return false;
			});
	}
}
