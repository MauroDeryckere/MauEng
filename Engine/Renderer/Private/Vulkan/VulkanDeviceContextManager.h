#ifndef MAUREN_VULKANDEVICECONTEXTMANAGER_H
#define MAUREN_VULKANDEVICECONTEXTMANAGER_H

#include "VulkanDeviceContext.h"

namespace MauRen
{
	class VulkanSurfaceContext;
	class VulkanInstanceContext;

	// This manager manages the device context, it's a singleton to prevent having to use dependency injection in all function
	// and/or having to store the device context as a member variable everywhere.
	// It also allows us to still extend the device context in the future if necessary to have e.g multiple logical devices
	class VulkanDeviceContextManager final : public MauCor::Singleton<VulkanDeviceContextManager>
	{
	public:
		bool Initialize(VulkanSurfaceContext* pSurfaceContext, VulkanInstanceContext* pInstanceContext);

		[[nodiscard]] VulkanDeviceContext* GetDeviceContext() const noexcept;

		bool Destroy();

		VulkanDeviceContextManager(VulkanDeviceContextManager const&) = delete;
		VulkanDeviceContextManager(VulkanDeviceContextManager&&) = delete;
		VulkanDeviceContextManager& operator=(VulkanDeviceContextManager const&) = delete;
		VulkanDeviceContextManager& operator=(VulkanDeviceContextManager const&&) = delete;
	private:
		friend class MauCor::Singleton<VulkanDeviceContextManager>;
		VulkanDeviceContextManager() = default;
		virtual ~VulkanDeviceContextManager() override;

		std::unique_ptr<VulkanDeviceContext> m_DeviceContext{ nullptr };
	};
}

#endif