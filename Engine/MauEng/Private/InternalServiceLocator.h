#ifndef MAUENG_INTERNALSERVICESLOCATOR_H
#define MAUENG_INTERNALSERVICESLOCATOR_H

#include "Renderer.h"
#include "RendererFactory.h"

namespace MauEng
{
	class InternalServiceLocator final
	{
	public:
		[[nodiscard]] static MauRen::Renderer& GetRenderer() { return (*m_pRenderer); }
		static void RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer);
	private:
		static std::unique_ptr<MauRen::Renderer> m_pRenderer;
	};


	#define RENDERER MauEng::InternalServiceLocator::GetRenderer()
}

#endif