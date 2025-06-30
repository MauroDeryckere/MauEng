#ifndef MAUENG_INTERNALSERVICESLOCATOR_H
#define MAUENG_INTERNALSERVICESLOCATOR_H

#include "Renderer.h"
#include "RendererFactory.h"
#include "GUI/GUILayer.h"

namespace MauEng
{
	class InternalServiceLocator final
	{
	public:
		[[nodiscard]] static MauRen::Renderer& GetRenderer() { return (*m_pRenderer); }
		static void RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer);

		[[nodiscard]] static GUILayer& GetGUILayer() { return (*m_pGUILayer); }
		static void RegisterGUILayer(std::unique_ptr<MauEng::GUILayer>&& pGUILayer);
	private:
		static std::unique_ptr<MauRen::Renderer> m_pRenderer;
		static std::unique_ptr<MauEng::GUILayer> m_pGUILayer;
	};


	#define RENDERER MauEng::InternalServiceLocator::GetRenderer()
}

#endif