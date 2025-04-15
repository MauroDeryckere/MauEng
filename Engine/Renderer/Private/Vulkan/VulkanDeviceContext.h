#ifndef MAUREN_VULKANDEVICECONTEXT_H
#define MAUREN_VULKANDEVICECONTEXT_H

#include "RendererPCH.h"
#include "VulkanInstanceContext.h"
#include "VulkanSurfaceContext.h"

namespace MauRen
{
	struct QueueFamilyIndices final
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		[[nodiscard]] bool IsComplete() const noexcept
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}

		[[nodiscard]] bool IsGraphicsPresentUnified() const noexcept
		{
			return (graphicsFamily.has_value() and presentFamily.has_value())
				and graphicsFamily.value() == presentFamily.value();
		}
	};

	class VulkanDeviceContext final
	{
	public:
		VulkanDeviceContext(VulkanSurfaceContext* pVulkanSurfaceContext, VulkanInstanceContext* pVulkanInstanceContext);
		~VulkanDeviceContext();

		[[nodiscard]] VkDevice GetLogicalDevice() const noexcept { return m_LogicalDevice; }
		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const noexcept { return m_PhysicalDevice; }

		[[nodiscard]] QueueFamilyIndices FindQueueFamilies() const noexcept;
		[[nodiscard]] bool IsUsingUnifiedGraphicsPresentQueue() const noexcept { return m_IsUsingUnifiedGraphicsPresentQueue; }

		[[nodiscard]] VkFormat FindDepthFormat() const noexcept;
		[[nodiscard]] static bool HasStencilComponent(VkFormat format) noexcept;

		[[nodiscard]] VkQueue GetGraphicsQueue() const noexcept { return m_IsUsingUnifiedGraphicsPresentQueue ? m_UnifiedGraphicsPresentQueue : m_GraphicsQueue; }
		[[nodiscard]] VkQueue GetPresentQueue() const noexcept { return m_IsUsingUnifiedGraphicsPresentQueue ? m_UnifiedGraphicsPresentQueue : m_PresentQueue; }

		[[nodiscard]] VkSampleCountFlagBits GetSampleCount() const noexcept { return m_MsaaSamples; }

		VulkanDeviceContext(VulkanDeviceContext const&) = delete;
		VulkanDeviceContext(VulkanDeviceContext&&) = delete;
		VulkanDeviceContext& operator=(VulkanDeviceContext const&) = delete;
		VulkanDeviceContext& operator=(VulkanDeviceContext&&) = delete;

	private:
		VulkanSurfaceContext* m_pVulkanSurfaceContext;
		VulkanInstanceContext* m_InstanceContext;

		// This object will be implicitly destroyed when the VkInstance is destroyed, so we won't need to do anything in cleanup.
		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice m_LogicalDevice{ VK_NULL_HANDLE };

		// Device queues are implicitly cleaned up when the device is destroyed, so we don't need to do anything in cleanup.
		VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue m_PresentQueue{ VK_NULL_HANDLE };

		bool m_IsUsingUnifiedGraphicsPresentQueue{ false };
		VkQueue m_UnifiedGraphicsPresentQueue{ VK_NULL_HANDLE };

		VkSampleCountFlagBits m_MsaaSamples;

		void SelectPhysicalDevice();
		void CreateLogicalDevice();

		// Checks if all requested extensions are available
		[[nodiscard]] bool CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice device);

		// Give a physical device a rating to allow automatically selecting the "best" option
		[[nodiscard]] uint32_t RateDeviceSuitability(VkPhysicalDevice device) const;

		// Is a physical device suitable for our application
		[[nodiscard]] bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);

		[[nodiscard]] QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const noexcept;

		[[nodiscard]] VkFormat FindSupportedFormat(std::vector<VkFormat> const& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

		[[nodiscard]] VkSampleCountFlagBits GetMaxUsableSampleCount() const noexcept;
	};
}

#endif // MAUREN_VULKANDEVICECONTEXT_H