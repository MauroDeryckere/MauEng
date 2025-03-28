#ifndef MAUREN_VULKANSWAPCHAINCONTEXT_H
#define MAUREN_VULKANSWAPCHAINCONTEXT_H

#include "RendererPCH.h"

#include "VulkanImage.h"

namespace MauRen
{
	struct SwapChainSupportDetails final
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanSurfaceContext;

	class VulkanSwapchainContext final
	{
	public:
		VulkanSwapchainContext() = default;
		~VulkanSwapchainContext() = default;

		void Initialize(GLFWwindow* pWindow, VulkanSurfaceContext* pVulkanSurfaceContext);
		void Destroy();

		// Query if swap chain is supported for a given physical device & window surface
		static SwapChainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface);

		[[nodiscard]] VkSwapchainKHR GetSwapchain() const noexcept { return m_SwapChain; }
		[[nodiscard]] std::vector<VulkanImage> const& GetImageViews() const noexcept { return m_SwapChainImages; }

		[[nodiscard]] VkExtent2D GetExtent() const noexcept { return m_SwapChainExtent; }
		[[nodiscard]] VkFormat GetImageFormat() const noexcept { return m_SwapChainImageFormat; }

		VulkanSwapchainContext(VulkanSwapchainContext const&) = delete;
		VulkanSwapchainContext(VulkanSwapchainContext&&) = delete;
		VulkanSwapchainContext& operator=(VulkanSwapchainContext const&) = delete;
		VulkanSwapchainContext& operator=(VulkanSwapchainContext&&) = delete;

	private:
		VulkanSurfaceContext* m_pSurfaceContext;

		VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };

		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		std::vector<VulkanImage> m_SwapChainImages;

		void CreateSwapchain(GLFWwindow* pWindow);
		void CreateImageViews();


		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes);
		static VkExtent2D ChooseSwapExtent(GLFWwindow* pWindow, VkSurfaceCapabilitiesKHR const& capabilities);
	};
}

#endif // MAUREN_VULKANSWAPCHAINCONTEXT_H