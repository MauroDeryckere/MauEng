#include "ServiceLocator.h"
#include "RendererFactory.h"

namespace MauEng
{
	std::unique_ptr<MauRen::DebugRenderer> ServiceLocator::m_pDebugRenderer{ MauRen::CreateDebugRenderer(true) };


	void ServiceLocator::RegisterDebugRenderer(std::unique_ptr<MauRen::DebugRenderer>&& pRenderer)
	{
		m_pDebugRenderer = ((!pRenderer) ? MauRen::CreateDebugRenderer(true) : std::move(pRenderer));
	}
}
