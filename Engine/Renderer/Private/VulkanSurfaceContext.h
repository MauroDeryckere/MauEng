#ifndef MAUREN_VULKANSURFACECONTEXT_H
#define MAUREN_VULKANSURFACECONTEXT_H

#include "RendererPCH.h"
#include "VulkanInstanceContext.h"
namespace MauRen
{
	class VulkanSurfaceContext final
	{
	public:
		VulkanSurfaceContext(VulkanInstanceContext const& vulkanInstanceContext, GLFWwindow* pWindow):
		m_VulkanInstanceContext{ vulkanInstanceContext }
		{
			if (glfwCreateWindowSurface(vulkanInstanceContext.GetInstance(), pWindow, nullptr, &m_WindowSurface) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create window surface!");
			}
		}
		~VulkanSurfaceContext()
		{
			// needs to be moved between instance & debyg ctxt
			vkDestroySurfaceKHR(m_VulkanInstanceContext.GetInstance(), m_WindowSurface, nullptr);
		}

		[[nodiscard]] VkSurfaceKHR GetWindowSurface() const noexcept { return m_WindowSurface; }

	private:
		VulkanInstanceContext const& m_VulkanInstanceContext;
		VkSurfaceKHR m_WindowSurface;



	};
}

#endif // MAUREN_VULKANSURFACECONTEXT_H