#include "RendererPCH.h"

#include "RendererFactory.h"
#include "Vulkan/VulkanRenderer.h"
#include "Vulkan/DebugRenderer/VulkanDebugRenderer.h"

namespace MauRen
{
	// Only vulkan renderer for now, factory pattern may be expanded in the future
	std::unique_ptr<Renderer> CreateVulkanRenderer(SDL_Window* pWindow)
	{
		return std::make_unique<VulkanRenderer>(pWindow);
	}

	std::unique_ptr<DebugRenderer> CreateVulkanDebugRenderer()
	{
		return std::make_unique<VulkanDebugRenderer>();
	}
}
