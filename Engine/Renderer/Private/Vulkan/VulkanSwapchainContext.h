#ifndef MAUREN_VULKANSWAPCHAINCONTEXT_H
#define MAUREN_VULKANSWAPCHAINCONTEXT_H

#include "RendererPCH.h"

#include "Assets/VulkanImage.h"

namespace MauRen
{
	struct SwapChainSupportDetails final
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	// Single colour for temp testing
	struct GBuffer final
	{
		VulkanImage color;
		//VulkanImage normal; // R8, G8 == Normal X; B8, A8 == Normal Y
		//VulkanImage albedo; // diffuse color information (RGB), A unused currently

		static std::array<VkFormat, 1> constexpr formats
		{
			VK_FORMAT_R8G8B8A8_SRGB
		};

		void Destroy()
		{
			color.Destroy();
		}
	};

	class VulkanSurfaceContext;
	class VulkanGraphicsPipelineContext;

	class VulkanSwapchainContext final
	{
	public:
		VulkanSwapchainContext() = default;
		~VulkanSwapchainContext() = default;

		// Initialize the swapchain
		void Initialize(SDL_Window* pWindow, VulkanSurfaceContext const* pVulkanSurfaceContext);

		// Reecreate the entire swapchain, this will destroy the previous swapchain first
		void ReCreate(SDL_Window* pWindow, VulkanGraphicsPipelineContext const* pGraphicsPipeline, VulkanSurfaceContext const* pVulkanSurfaceContext);

		void Destroy();

		// Query if swap chain is supported for a given physical device & window surface
		static [[nodiscard]] SwapChainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface);

		[[nodiscard]] VkSwapchainKHR GetSwapchain() const noexcept { return m_SwapChain; }
		[[nodiscard]] std::vector<VulkanImage> const& GetSwapchainImages() const noexcept { return m_SwapChainImages; }
		[[nodiscard]] std::vector<VulkanImage>& GetSwapchainImages()noexcept { return m_SwapChainImages; }
		[[nodiscard]] VkFormat GetImageFormat() const noexcept { return m_SwapChainImageFormat; }

		[[nodiscard]] VulkanImage const& GetColorImage(uint32_t frame) const noexcept { return m_ColorImages[frame]; }
		[[nodiscard]] VulkanImage& GetColorImage(uint32_t frame) noexcept { return m_ColorImages[frame]; }

		[[nodiscard]] VulkanImage const& GetDepthImage(uint32_t frame) const noexcept { return m_DepthImages[frame]; }
		[[nodiscard]] VulkanImage& GetDepthImage(uint32_t frame) noexcept { return m_DepthImages[frame]; }

		[[nodiscard]] GBuffer const& GetGBuffer(uint32_t frame) const noexcept { return m_GBuffers[frame]; }
		[[nodiscard]] GBuffer& GetGBuffer(uint32_t frame) noexcept { return m_GBuffers[frame]; }

		[[nodiscard]] VkExtent2D GetExtent() const noexcept { return m_SwapChainExtent; }

		VulkanSwapchainContext(VulkanSwapchainContext const&) = delete;
		VulkanSwapchainContext(VulkanSwapchainContext&&) = delete;
		VulkanSwapchainContext& operator=(VulkanSwapchainContext const&) = delete;
		VulkanSwapchainContext& operator=(VulkanSwapchainContext&&) = delete;

	private:
		VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };

		VkFormat m_SwapChainImageFormat{};
		VkExtent2D m_SwapChainExtent{};

		// final presentation imgs
		std::vector<VulkanImage> m_SwapChainImages{};

		// written to in depth prepass, used everywhere else
		std::vector<VulkanImage> m_DepthImages{};
		// Written to in lightpas
		std::vector<VulkanImage> m_ColorImages{};

		// gbuffer pass
		std::vector<GBuffer> m_GBuffers{};

		void CreateSwapchain(SDL_Window* pWindow, VulkanSurfaceContext const * pVulkanSurfaceContext);
		void CreateImageViews();

		static [[nodiscard]] VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats);
		static [[nodiscard]] VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes);
		static [[nodiscard]] VkExtent2D ChooseSwapExtent(SDL_Window* pWindow, VkSurfaceCapabilitiesKHR const& capabilities);

		void CreateColorResources();
		void CreateDepthResources();

		void CreateGBuffers();
	};
}

#endif // MAUREN_VULKANSWAPCHAINCONTEXT_H