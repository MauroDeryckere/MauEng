#ifndef MAUREN_RENDERER_H
#define MAUREN_RENDERER_H

#include "Mesh.h"
#include "MeshInstance.h"

struct SDL_Window;

namespace MauRen
{
	class Renderer
	{
	public:
		virtual ~Renderer() = default;

		virtual void Init() = 0;
		virtual void Destroy() = 0;

		virtual void Render(glm::mat4 const& view, glm::mat4 const& proj) = 0;

		virtual void ResizeWindow() = 0;

		// Upload model to the GPU
		virtual void UpLoadModel(Mesh& mesh) = 0;

		Renderer(Renderer const&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer const&) = delete;
		Renderer& operator=(Renderer&&) = delete;

	protected:
		explicit Renderer(SDL_Window* pWindow, class DebugRenderer&) {}
	};

	class NullRenderer final : public Renderer
	{
	public:
		explicit NullRenderer(SDL_Window* pWindow, class DebugRenderer& debugRenderer) :
			Renderer{ pWindow, debugRenderer }
		{
		}
		virtual ~NullRenderer() override = default;

		virtual void Init() override {}
		virtual void Destroy() override {}

		virtual void Render(glm::mat4 const&, glm::mat4 const&) override {}
		virtual void ResizeWindow() override {}

		virtual void UpLoadModel(Mesh&) override {}

		NullRenderer(NullRenderer const&) = delete;
		NullRenderer(NullRenderer&&) = delete;
		NullRenderer& operator=(NullRenderer const&) = delete;
		NullRenderer& operator=(NullRenderer&&) = delete;
	};
}

#endif // MAUREN_RENDERER_H