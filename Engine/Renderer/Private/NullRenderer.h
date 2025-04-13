#ifndef MAUREN_NULLRENDERER
#define MAUREN_NULLRENDERER

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

		virtual void UpLoadModel(Mesh&) override {}

		NullRenderer(NullRenderer const&) = delete;
		NullRenderer(NullRenderer&&) = delete;
		NullRenderer& operator=(NullRenderer const&) = delete;
		NullRenderer& operator=(NullRenderer&&) = delete;
	};
}

#endif