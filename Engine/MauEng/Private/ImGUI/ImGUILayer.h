#ifndef MAUENG_IMGUILAYER_H
#define MAUENG_IMGUILAYER_H

namespace MauEng
{
	class SDLWindow;

	class ImGUILayer final
	{
	public:
		void Init(SDLWindow* pWindow);
		void Destroy();

		void BeginFrame();

		void Render(MauEng::Camera const* cam);

		void EndFrame();

	private:
	};
}

#endif