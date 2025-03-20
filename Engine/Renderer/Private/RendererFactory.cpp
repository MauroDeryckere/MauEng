#include "RendererPCH.h"

#include "RendererFactory.h"
#include "VulkanRenderer.h"

namespace MauRen {
	// Only vulkan renderer for now, factory pattern may be expanded in the future
	std::unique_ptr<Renderer> CreateRenderer(GLFWwindow* pWindow)
	{
		return std::make_unique<VulkanRenderer>(pWindow);
	}
}