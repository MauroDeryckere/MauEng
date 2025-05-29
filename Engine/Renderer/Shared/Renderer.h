#ifndef MAUREN_RENDERER_H
#define MAUREN_RENDERER_H

#include "MeshInstance.h"

namespace MauEng
{
	struct CStaticMesh;
}

struct SDL_Window;

namespace MauRen
{
	class Renderer
	{
	public:
		virtual ~Renderer() = default;

		virtual void Init() = 0;
		virtual void Destroy() = 0;

		virtual void Render(glm::mat4 const& view, glm::mat4 const& proj, glm::vec2 const& screenSize) = 0;

		virtual void ResizeWindow() = 0;

		virtual void QueueDraw(glm::mat4 const& transformMat, MauEng::CStaticMesh const& mesh) = 0;
		virtual [[nodiscard]] uint32_t LoadOrGetMeshID(char const* path) = 0;

		Renderer(Renderer const&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer const&) = delete;
		Renderer& operator=(Renderer&&) = delete;
																			
	protected:
		explicit Renderer(SDL_Window* pWindow, class DebugRenderer&) {}
	};
}

#endif // MAUREN_RENDERER_H