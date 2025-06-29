#ifndef MAUENG_GUILAYER_H
#define MAUENG_GUILAYER_H

namespace MauEng
{
	class GUILayer
	{
	public:
		GUILayer() = default;
		virtual ~GUILayer() = default;

		virtual void Init(class SDLWindow* pWindow) = 0;
		virtual void Destroy() = 0;
		virtual void BeginFrame() = 0;
		virtual void Render(class Camera const* cam) = 0;
		virtual void EndFrame() = 0;

		GUILayer(GUILayer const&) = delete;
		GUILayer(GUILayer&&) = delete;
		GUILayer& operator=(GUILayer const&) = delete;
		GUILayer& operator=(GUILayer&&) = delete;
	};
}

#endif