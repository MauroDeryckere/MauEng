#ifndef RENDERRENDERERFACTORY_H
#define RENDERRENDERERFACTORY_H

#include <memory>
#include "Renderer.h"
#include "DebugRenderer.h"

namespace MauRen
{
	std::unique_ptr<Renderer> CreateVulkanRenderer(SDL_Window* pWindow, DebugRenderer& debugRenderer);
}

#endif // RENDERRENDERERFACTORY_H