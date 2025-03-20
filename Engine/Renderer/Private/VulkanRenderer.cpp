#include "VulkanRenderer.h"

namespace MauRen
{
	VulkanRenderer::VulkanRenderer(GLFWwindow* pWindow) :
		Renderer{ pWindow },
		m_InstanceContext{ std::make_unique<VulkanInstanceContext>() },
		m_SurfaceContext{ std::make_unique<VulkanSurfaceContext>(m_InstanceContext.get(), pWindow) },
		m_DebugContext{ std::make_unique<VulkanDebugContext>(m_InstanceContext.get()) },
		m_DeviceContext{ std::make_unique<VulkanDeviceContext>(m_SurfaceContext.get(), m_InstanceContext.get())},
		m_SwapChainContext{ std::make_unique<VulkanSwapchainContext>(pWindow, m_SurfaceContext.get(), m_DeviceContext.get()) }

	{
	}

	VulkanRenderer::~VulkanRenderer()
	{
	}

	void VulkanRenderer::Render()
	{
		//TODO
	}
}


