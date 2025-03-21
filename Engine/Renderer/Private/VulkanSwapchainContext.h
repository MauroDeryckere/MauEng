#ifndef MAUREN_VULKANSWAPCHAINCONTEXT_H
#define MAUREN_VULKANSWAPCHAINCONTEXT_H

#include "RendererPCH.h"

namespace MauRen
{
	struct SwapChainSupportDetails final
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanDeviceContext;
	class VulkanSurfaceContext;

	class VulkanSwapchainContext final
	{
	public:
		VulkanSwapchainContext(GLFWwindow* pWindow, VulkanSurfaceContext* pVulkanSurfaceContext, VulkanDeviceContext* pVulkanDeviceContext);
		~VulkanSwapchainContext();

		// Query if swap chain is supported for a given physical device & window surface
		static SwapChainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface);
		[[nodiscard]] VkExtent2D GetExtent() const noexcept { return m_SwapChainExtent; }

		VulkanSwapchainContext(VulkanSwapchainContext const&) = delete;
		VulkanSwapchainContext(VulkanSwapchainContext&&) = delete;
		VulkanSwapchainContext& operator=(VulkanSwapchainContext const&) = delete;
		VulkanSwapchainContext& operator=(VulkanSwapchainContext&&) = delete;

	private:
		VulkanSurfaceContext* m_pSurfaceContext;
		VulkanDeviceContext* m_pDeviceContext;

		VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };

		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		std::vector<VkImageView> m_SwapChainImageViews;

		void CreateSwapchain(GLFWwindow* pWindow);
		void CreateImageViews();


		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes);
		static VkExtent2D ChooseSwapExtent(GLFWwindow* pWindow, VkSurfaceCapabilitiesKHR const& capabilities);
	};
}

#endif // MAUREN_VULKANSWAPCHAINCONTEXT_H