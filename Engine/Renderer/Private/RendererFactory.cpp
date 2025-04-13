#include "RendererPCH.h"

#include "RendererFactory.h"
#include "Vulkan/VulkanRenderer.h"

#include "NullRenderer.h"

#include "DebugRenderer/InternalDebugRenderer.h"
#include "DebugRenderer/NullDebugRenderer.h"

namespace MauRen
{
	std::unique_ptr<Renderer> CreateVulkanRenderer(SDL_Window* pWindow, DebugRenderer& debugRenderer)
	{
		return std::make_unique<VulkanRenderer>(pWindow, debugRenderer);
	}

	std::unique_ptr<Renderer> CreateNullRenderer()
	{
		return std::make_unique<NullRenderer>(nullptr, *CreateDebugRenderer(true).get());
	}

	std::unique_ptr<DebugRenderer> CreateDebugRenderer(bool isNull)
	{
		if (isNull)
		{
			return std::make_unique<NullDebugRenderer>();
		}

		return std::make_unique<InternalDebugRenderer>();
	}

}
