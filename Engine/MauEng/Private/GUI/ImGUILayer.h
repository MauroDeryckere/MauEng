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

		void Render(class Camera const* cam) override;

		void EndFrame() override;
	private:
	};
}

#endif