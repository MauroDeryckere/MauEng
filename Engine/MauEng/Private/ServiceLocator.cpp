#include "ServiceLocator.h"
#include "RendererFactory.h"

namespace MauEng
{
	std::unique_ptr<MauRen::Renderer> ServiceLocator::m_pRenderer{ MauRen::CreateNullRenderer() };
	std::unique_ptr<MauRen::DebugRenderer> ServiceLocator::m_pDebugRenderer{ MauRen::CreateDebugRenderer(true) };

	void ServiceLocator::RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer)
	{
		m_pRenderer = ((!pRenderer) ? MauRen::CreateNullRenderer() : std::move(pRenderer));
	}

	void ServiceLocator::RegisterDebugRenderer(std::unique_ptr<MauRen::DebugRenderer>&& pRenderer)
	{
		m_pDebugRenderer = ((!pRenderer) ? MauRen::CreateDebugRenderer(true) : std::move(pRenderer));
	}
}
