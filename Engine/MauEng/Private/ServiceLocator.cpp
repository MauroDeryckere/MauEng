#include "ServiceLocator.h"

#include "NullDebugRenderer.h"

namespace MauEng
{
	std::unique_ptr<MauRen::Renderer> ServiceLocator::m_pRenderer{ std::make_unique<MauRen::NullRenderer>(nullptr, *std::make_unique<MauRen::NullDebugRenderer>()) };
	std::unique_ptr<MauRen::DebugRenderer> ServiceLocator::m_pDebugRenderer{ std::make_unique<MauRen::NullDebugRenderer>() };

	void ServiceLocator::RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer)
	{
		m_pRenderer = ((!pRenderer) ? std::make_unique<MauRen::NullRenderer>(nullptr, *std::make_unique<MauRen::NullDebugRenderer>()) : std::move(pRenderer));
	}

	void ServiceLocator::RegisterDebugRenderer(std::unique_ptr<MauRen::DebugRenderer>&& pRenderer)
	{
		m_pDebugRenderer = ((!pRenderer) ? std::make_unique<MauRen::NullDebugRenderer>() : std::move(pRenderer));
	}
}
