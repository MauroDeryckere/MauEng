#include "VulkanDebugContext.h"

namespace MauRen
{
	VulkanDebugContext::VulkanDebugContext(VulkanInstanceContext* vulkanInstanceContext) :
		m_pVulkanInstanceContext{ vulkanInstanceContext }

	{
		SetupDebugMessenger();
	}

	VulkanDebugContext::~VulkanDebugContext()
	{
		DestroyDebugUtilsMessengerEXT(m_pVulkanInstanceContext->GetInstance(), m_DebugMessenger, nullptr);
	}

	void VulkanDebugContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugContext::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void VulkanDebugContext::SetupDebugMessenger()
	{
		if constexpr (!ENABLE_VULKAN_VALIDATION_LAYERS) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(m_pVulkanInstanceContext->GetInstance(), &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	VkResult VulkanDebugContext::CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const* pCreateInfo, VkAllocationCallbacks const* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		if (auto func{ (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT") })
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void VulkanDebugContext::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks const* pAllocator)
	{
		// ! Make sure that this function is either a static class function or a function outside the class.

		if (auto func{ (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT") })
		{
			func(instance, debugMessenger, pAllocator);
		}
	}



}
