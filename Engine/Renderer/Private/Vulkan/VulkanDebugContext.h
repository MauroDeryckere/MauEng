#ifndef MAUREN_VULKANDEBUGCONTEXT_H
#define MAUREN_VULKANDEBUGCONTEXT_H

#include "RendererPCH.h"
#include "VulkanInstanceContext.h"

namespace MauRen
{
	class VulkanDebugContext final
	{
	public:
		VulkanDebugContext(VulkanInstanceContext* pVulkanInstanceContext);
		~VulkanDebugContext();

		VulkanDebugContext(VulkanDebugContext const&) = delete;
		VulkanDebugContext(VulkanDebugContext&&) = delete;
		VulkanDebugContext& operator=(VulkanDebugContext const&) = delete;
		VulkanDebugContext& operator=(VulkanDebugContext&&) = delete;

		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		[[nodiscard]] static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	private:
		// Need to store the context we use to destroy it later
		VulkanInstanceContext* m_pVulkanInstanceContext;

		VkDebugUtilsMessengerEXT m_DebugMessenger { VK_NULL_HANDLE };

		void SetupDebugMessenger();

		static [[nodiscard]] VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const* pCreateInfo, VkAllocationCallbacks const* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks const* pAllocator);

	};
}

#endif // MAUREN_VULKANDEBUGCONTEXT_H