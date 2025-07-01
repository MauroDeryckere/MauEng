#ifndef MAUENG_IMGUILAYER_H
#define MAUENG_IMGUILAYER_H

#include "GUILayer.h"

namespace MauEng
{
	class SDLWindow;

	class ImGUILayer final : public GUILayer
	{
	public:
		void Init(SDLWindow* pWindow) override;
		void Destroy() override;

		void BeginFrame() override;

		void Render(class Scene* scene, SDLWindow* pWindow) override;

		void EndFrame() override;
	private:
		static void RenderDebugText(class Scene* scene, SDLWindow* pWindow);
		static void RenderConsoleOutput();
		static void RenderRendererInfo();
	};
}

#endif