#ifndef MAUREN_NULLRENDERER
#define MAUREN_NULLRENDERER

#include "RendererIdentifiers.h"

namespace MauEng
{
	struct CStaticMesh;
	class Camera;
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
		virtual void InitImGUI() override {}

		virtual void Destroy() override {}
		virtual void DestroyImGUI() override {}

		virtual void BeginImGUIFrame() override {}
		virtual void Render(MauEng::Camera const*) override {}
		virtual void EndImGUIFrame() override {}

		virtual void ResizeWindow() override {}

		virtual void QueueDraw(glm::mat4 const&, MauEng::CStaticMesh const&) override {}
		virtual uint32_t LoadOrGetMeshID(char const*) override { return INVALID_MESH_ID; }

		virtual void SetSceneAABBOverride(glm::vec3 const&, glm::vec3 const&) override {}
		virtual void PreLightQueue(glm::mat4 const&) override {}
		virtual uint32_t CreateLight() override { return INVALID_LIGHT_ID; }
		virtual void QueueLight(MauEng::CLight const&) override {}

		NullRenderer(NullRenderer const&) = delete;
		NullRenderer(NullRenderer&&) = delete;
		NullRenderer& operator=(NullRenderer const&) = delete;
		NullRenderer& operator=(NullRenderer&&) = delete;
	};
}

#endif