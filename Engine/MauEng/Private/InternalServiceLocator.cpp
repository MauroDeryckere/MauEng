#include "InternalServiceLocator.h"
#include "RendererFactory.h"

#include "GUI/NullGUILayer.h"

#include <memory>

namespace MauEng
{
	std::unique_ptr<MauRen::Renderer> InternalServiceLocator::m_pRenderer{ MauRen::CreateNullRenderer() };
	std::unique_ptr<MauEng::GUILayer> InternalServiceLocator::m_pGUILayer{ std::make_unique<NullGUILayer>() };
	void InternalServiceLocator::RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer)
	{
		m_pRenderer = ((!pRenderer) ? MauRen::CreateNullRenderer() : std::move(pRenderer));
	}

	void InternalServiceLocator::RegisterGUILayer(std::unique_ptr<MauEng::GUILayer>&& pGUILayer)
	{
		m_pGUILayer = ((!pGUILayer) ? std::make_unique<NullGUILayer>() : std::move(pGUILayer));
	}
}
