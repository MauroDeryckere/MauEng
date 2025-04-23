#ifndef MAUREN_NULLRENDERER
#define MAUREN_NULLRENDERER

#include "RendererIdentifiers.h"

namespace MauEng
{
	struct CStaticMesh;
}

namespace MauRen
{
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

		virtual void QueueDraw(glm::mat4 const&, MauEng::CStaticMesh const&) override {}
		virtual uint32_t LoadOrGetMeshID(char const*) override { return INVALID_MESH_ID; }

		NullRenderer(NullRenderer const&) = delete;
		NullRenderer(NullRenderer&&) = delete;
		NullRenderer& operator=(NullRenderer const&) = delete;
		NullRenderer& operator=(NullRenderer&&) = delete;
	};
}

#endif