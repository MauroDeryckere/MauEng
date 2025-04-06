#ifndef RENDERRENDERERFACTORY_H
#define RENDERRENDERERFACTORY_H

#include <memory>
#include "Renderer.h"
#include "DebugRenderer.h"

namespace MauRen
{
	std::unique_ptr<Renderer> CreateVulkanRenderer(SDL_Window* pWindow);
	std::unique_ptr<DebugRenderer> CreateVulkanDebugRenderer(Renderer& pRenderer);
}

#endif // RENDERRENDERERFACTORY_H