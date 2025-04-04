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
	class VulkanGraphicsPipeline;

	class VulkanSwapchainContext final
	{
	public:
		VulkanSwapchainContext() = default;
		~VulkanSwapchainContext() = default;

		// Initialize the swapchain
		void Initialize(SDL_Window* pWindow, VulkanSurfaceContext const * pVulkanSurfaceContext);
		// Intialize the depth image, colour image and create frames, this must be done after the graphics pipeline creation
		void InitializeResourcesAndCreateFrames(VulkanGraphicsPipeline const* pGraphicsPipeline);

		// Reecreate the entire swapchain, this will destroy the previous swapchain first
		void ReCreate(SDL_Window* pWindow, VulkanGraphicsPipeline const* pGraphicsPipeline, VulkanSurfaceContext const* pVulkanSurfaceContext);

		void Destroy();

		// Query if swap chain is supported for a given physical device & window surface
		static SwapChainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface);

		[[nodiscard]] VkSwapchainKHR GetSwapchain() const noexcept { return m_SwapChain; }
		[[nodiscard]] std::vector<VulkanImage> const& GetImageViews() const noexcept { return m_SwapChainImages; }

		[[nodiscard]] VkExtent2D GetExtent() const noexcept { return m_SwapChainExtent; }
		[[nodiscard]] VkFormat GetImageFormat() const noexcept { return m_SwapChainImageFormat; }

		[[nodiscard]] VkFramebuffer GetSwapchainFrameBuffer(uint32_t imageIndex) const noexcept;

		VulkanSwapchainContext(VulkanSwapchainContext const&) = delete;
		VulkanSwapchainContext(VulkanSwapchainContext&&) = delete;
		VulkanSwapchainContext& operator=(VulkanSwapchainContext const&) = delete;
		VulkanSwapchainContext& operator=(VulkanSwapchainContext&&) = delete;

	private:
		VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };

		VkFormat m_SwapChainImageFormat{};
		VkExtent2D m_SwapChainExtent{};

		std::vector<VulkanImage> m_SwapChainImages{};

		VulkanImage m_DepthImage{};
		VulkanImage m_ColorImage{};

		std::vector<VkFramebuffer> m_SwapChainFrameBuffers{};

		void CreateSwapchain(SDL_Window* pWindow, VulkanSurfaceContext const * pVulkanSurfaceContext);
		void CreateImageViews();

		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes);
		static VkExtent2D ChooseSwapExtent(SDL_Window* pWindow, VkSurfaceCapabilitiesKHR const& capabilities);

		void CreateColorResources();
		void CreateDepthResources();

		void CreateFrameBuffers(VulkanGraphicsPipeline const* pGraphicsPipeline);
	};
}

#endif // MAUREN_VULKANSWAPCHAINCONTEXT_H