#ifndef MAUENG_NULLGUILAYER_H
#define MAUENG_NULLGUILAYER_H

#include "GuiLayer.h"

namespace MauEng
{
	class NullGUILayer : public GUILayer
	{
	public:
		NullGUILayer() = default;
		virtual ~NullGUILayer() override = default;

		virtual void Init(class SDLWindow* pWindow) override {}
		virtual void Destroy() override {}

		virtual void BeginFrame() override {}
		virtual void Render(class Scene*, class SDLWindow*) override {}
		virtual void EndFrame() override {}

		NullGUILayer(NullGUILayer const&) = delete;
		NullGUILayer(NullGUILayer&&) = delete;
		NullGUILayer& operator=(NullGUILayer const&) = delete;
		NullGUILayer& operator=(NullGUILayer&&) = delete;
	};
}

#endif