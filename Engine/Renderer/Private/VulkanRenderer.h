#ifndef MAUREN_VULKANRENDERER_H
#define MAUREN_VULKANRENDERER_H

#include "RendererPCH.h"

#include "Renderer.h"

#include "VulkanInstanceContext.h"
#include "VulkanSurfaceContext.h"
#include "VulkanDebugContext.h"


#include "VulkanQueueManager.h"
#include "VulkanDeviceContext.h"
#include "VulkanSwapchainContext.h"

namespace MauRen
{
	class VulkanRenderer final : public Renderer
	{
	public:
		explicit VulkanRenderer(GLFWwindow* pWindow);
		virtual ~VulkanRenderer() override;

		virtual void Initialize(GLFWwindow* pWindow) override;
		virtual void Render() override;


		VulkanRenderer(VulkanRenderer const&) = delete;
		VulkanRenderer(VulkanRenderer&&) = delete;
		VulkanRenderer& operator=(VulkanRenderer const&) = delete;
		VulkanRenderer& operator=(VulkanRenderer&&) = delete;

	private:
		//MUST BE MOVED LATER TODO
		GLFWwindow* m_pWindow;

		VulkanInstanceContext m_InstanceContext;
		VulkanSurfaceContext m_SurfaceContext;
		VulkanDebugContext m_DebugContext;

		// Surface ctxt
		// ...


		// This object will be implicitly destroyed when the VkInstance is destroyed, so we won't need to do anything new in the cleanup function.
		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };

		VkDevice m_LogicalDevice;

		// Device queues are implicitly cleaned up when the device is destroyed, so we don't need to do anything in cleanup.
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		bool m_IsUsingUnifiedGraphicsPresentQueue{ false };
		VkQueue m_UnifiedGraphicsPresentQueue;

		VkSwapchainKHR m_SwapChain;

		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		std::vector<VkImageView> m_SwapChainImageViews;


		struct QueueFamilyIndices final
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			[[nodiscard]] bool IsComplete() const noexcept
			{
				return graphicsFamily.has_value() and presentFamily.has_value();
			}

			[[nodiscard]] bool IsGraphicsPresentUnified() const noexcept
			{
				return (graphicsFamily.has_value() and presentFamily.has_value())
					and graphicsFamily.value() == presentFamily.value();
			}
		};

		[[nodiscard]] QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
		{
			QueueFamilyIndices indices{};

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());


			for (uint32_t i{ 0 }; i < static_cast<uint32_t>(queueFamilies.size()); ++i)
			{
				VkBool32 presentSupport{ false };
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_SurfaceContext.GetWindowSurface(), &presentSupport);

				if (presentSupport)
				{
					indices.presentFamily = i;
				}

				if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					indices.graphicsFamily = i;
				}

				// Temporary
				if (indices.IsComplete())
				{
					break;
				}
			}
			return indices;
		}

		// Checks if all requested extensions are available
		[[nodiscard]] bool CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
		{
			std::cout << "\n";

			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			return std::ranges::all_of(DEVICE_EXTENSIONS, [&](std::string_view ext)
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


		struct SwapChainSupportDetails final
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device)
		{
			SwapChainSupportDetails details;

			// basic surface capabilities
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_SurfaceContext.GetWindowSurface(), &details.capabilities);

			// querying the supported surface formats
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_SurfaceContext.GetWindowSurface(), &formatCount, nullptr);

			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_SurfaceContext.GetWindowSurface(), &formatCount, details.formats.data());
			}

			// querying the supported presentation modes
			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_SurfaceContext.GetWindowSurface(), &presentModeCount, nullptr);

			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_SurfaceContext.GetWindowSurface(), &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		// Is a physical device suitable for our application
		[[nodiscard]] bool IsPhysicalDeviceSuitable(VkPhysicalDevice device)
		{
			QueueFamilyIndices const indices{ FindQueueFamilies(device) };

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			bool const extensionsSupported{ CheckPhysicalDeviceExtensionSupport(device) };

			bool swapChainAdequate{ false };
			if (extensionsSupported)
			{
				SwapChainSupportDetails const swapChainSupport{ QuerySwapChainSupport(device) };
				swapChainAdequate = not swapChainSupport.formats.empty() && not swapChainSupport.presentModes.empty();
			}

			return indices.IsComplete() and extensionsSupported and swapChainAdequate;
		}

		// Give a physical device a rating to allow automatically selecting the "best" option
		[[nodiscard]] uint32_t RateDeviceSuitability(VkPhysicalDevice device)
		{
			QueueFamilyIndices const indices{ FindQueueFamilies(device) };

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			uint32_t score{ 0 };
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				score += 1000;
			}

			score += deviceProperties.limits.maxImageDimension2D;

			// Prefer devices with unified graphics & present queue family
			if (indices.IsGraphicsPresentUnified())
			{
				score += 500;
			}

			return score;
		}


		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats)
		{
			// For the color space we'll use SRGB if it is available.
			// Because it results in more accurate perceived colors.
			// It is also pretty much the standard color space for images.
			for (auto const& availableFormat : availableFormats)
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return availableFormat;
				}
			}
			// If that also fails then we could start ranking the available formats based on how "good" they are.
			// In most cases it's okay to just settle with the first format that is specified.
			// TODO Implement ranking
			return availableFormats[0];
		}

		VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes)
		{
			for (auto const& availablePresentMode : availablePresentModes)
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return availablePresentMode;
				}
			}

			// Guaruanteed to be available
			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR const& capabilities)
		{
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				return capabilities.currentExtent;
			}

			int width, height;
			glfwGetFramebufferSize(m_pWindow, &width, &height);

			VkExtent2D actualExtent
			{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}

		void InitVulkan();

		//void CreateVulkanInstance();
		//void SetupDebugMessenger();
		//void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();

		void Cleanup();
	};
}

#endif
