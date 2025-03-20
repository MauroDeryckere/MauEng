#ifndef MAUREN_VULKANINSTANCECONTEXT_H
#define MAUREN_VULKANINSTANCECONTEXT_H

#include "RendererPCH.h"

namespace MauRen
{
	class VulkanInstanceContext final
	{
	public:
		VulkanInstanceContext();
		~VulkanInstanceContext();

		VulkanInstanceContext(VulkanInstanceContext const&) = delete;
		VulkanInstanceContext(VulkanInstanceContext&&) = delete;
		VulkanInstanceContext& operator=(VulkanInstanceContext const&) = delete;
		VulkanInstanceContext& operator=(VulkanInstanceContext&&) = delete;

		[[nodiscard]] VkInstance GetInstance() const noexcept { return m_Instance; }

	private:
		VkInstance m_Instance;

		void CreateVulkanInstance();

		// Checks if all requested validation layers are available
		[[nodiscard]] bool CheckValidationLayerSupport();

		// Returns the required extensions for the instance
		[[nodiscard]] std::vector<char const*> GetRequiredExtensions();

		// Checks if all requested extensions are available
		[[nodiscard]] bool CheckInstanceExtensionsSupport(uint32_t extensionCount, std::vector<char const*> const& extensions, std::vector<VkExtensionProperties> const& availableExtensions);
	};
}

#endif // MAUREN_VULKANINSTANCECONTEXT_H