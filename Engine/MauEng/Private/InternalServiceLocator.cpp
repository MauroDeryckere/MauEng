#include "InternalServiceLocator.h"
#include "RendererFactory.h"

#include <memory>

namespace MauEng
{
	std::unique_ptr<MauRen::Renderer> InternalServiceLocator::m_pRenderer{ MauRen::CreateNullRenderer() };
	void InternalServiceLocator::RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer)
	{
		m_pRenderer = ((!pRenderer) ? MauRen::CreateNullRenderer() : std::move(pRenderer));
	}
}