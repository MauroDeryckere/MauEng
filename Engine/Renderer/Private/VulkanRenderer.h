#ifndef MAUREN_VULKANRENDERER_H
#define MAUREN_VULKANRENDERER_H

#include "RendererPCH.h"

#include "Renderer.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"



namespace MauRen
{
#ifdef NDEBUG
	bool constexpr ENABLE_VULKAN_VALIDATION_LAYERS{ false };
#else
	bool constexpr ENABLE_VULKAN_VALIDATION_LAYERS{ true };
#endif

// Using a unified queue may result in less overhead and a performance boost
bool constexpr FORCE_SEPARATE_GRAPHICS_PRESENT_QUEUES{ false };

bool constexpr AUTO_SELECT_PHYSICAL_DEVICE{ true };

	class VulkanRenderer final : public Renderer
	{
	public:
		VulkanRenderer();
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

		// globals, to be moved to the renderer in the future
		VkInstance m_Instance;
		// This object will be implicitly destroyed when the VkInstance is destroyed, so we won't need to do anything new in the cleanup function.
		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };

		VkDevice m_LogicalDevice;

		// Device queues are implicitly cleaned up when the device is destroyed, so we don't need to do anything in cleanup.
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;



		bool m_IsUsingUnifiedGraphicsPresentQueue{ false };
		VkQueue m_UnifiedGraphicsPresentQueue;


		VkDebugUtilsMessengerEXT m_DebugMessenger;

		VkSurfaceKHR m_WindowSurface;

		VkSwapchainKHR m_SwapChain;

		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		std::vector<VkImageView> m_SwapChainImageViews;


		std::vector<char const*> const VULKAN_VALIDATION_LAYERS
		{
			"VK_LAYER_KHRONOS_validation",
		};

		std::vector<char const*> const DEVICE_EXTENSIONS
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};


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
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_WindowSurface, &presentSupport);

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

		[[nodiscard]] static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
		{
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}

		// Checks if all requested validation layers are available
		[[nodiscard]] bool CheckValidationLayerSupport()
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

		[[nodiscard]] std::vector<char const*> GetRequiredExtensions()
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

		// Checks if all requested extensions are available
		[[nodiscard]] bool CheckInstanceExtensionsSupport(uint32_t extensionCount, std::vector<char const*> const& extensions, std::vector<VkExtensionProperties> const& availableExtensions)
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

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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

		[[nodiscard]] VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const* pCreateInfo, VkAllocationCallbacks const* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			if (auto func{ (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT") })
			{
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}

			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks const* pAllocator)
		{
			// ! Make sure that this function is either a static class function or a function outside the class.

			if (auto func{ (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT") })
			{
				func(instance, debugMessenger, pAllocator);
			}
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
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_WindowSurface, &details.capabilities);

			// querying the supported surface formats
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_WindowSurface, &formatCount, nullptr);

			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_WindowSurface, &formatCount, details.formats.data());
			}

			// querying the supported presentation modes
			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_WindowSurface, &presentModeCount, nullptr);

			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_WindowSurface, &presentModeCount, details.presentModes.data());
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

		void CreateVulkanInstance();
		void SetupDebugMessenger();
		void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();

		void Cleanup();
	};
}

#endif
