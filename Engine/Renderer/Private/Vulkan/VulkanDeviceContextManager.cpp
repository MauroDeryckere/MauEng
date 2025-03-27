#include "VulkanDeviceContextManager.h"

namespace MauRen
{
	bool VulkanDeviceContextManager::Initialize(VulkanSurfaceContext* pSurfaceContext,VulkanInstanceContext* pInstanceContext)
	{
		m_DeviceContext = std::make_unique<VulkanDeviceContext>(pSurfaceContext, pInstanceContext);

		return true;
	}

	VulkanDeviceContext* VulkanDeviceContextManager::GetDeviceContext() const noexcept
	{
		return m_DeviceContext.get();
	}

	bool VulkanDeviceContextManager::Destroy()
	{
		m_DeviceContext = nullptr;
		return true;
	}

	VulkanDeviceContextManager::~VulkanDeviceContextManager()
	{
		Destroy();
	}
}