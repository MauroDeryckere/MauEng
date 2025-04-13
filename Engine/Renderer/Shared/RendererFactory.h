#ifndef RENDERRENDERERFACTORY_H
#define RENDERRENDERERFACTORY_H

#include <memory>

namespace MauRen
{
	class Renderer;
	class DebugRenderer;

	std::unique_ptr<Renderer> CreateVulkanRenderer(SDL_Window* pWindow, DebugRenderer& debugRenderer);
	std::unique_ptr<Renderer> CreateNullRenderer();
	std::unique_ptr<DebugRenderer> CreateDebugRenderer(bool isNull);
}

#endif // RENDERRENDERERFACTORY_H