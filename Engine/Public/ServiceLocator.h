#ifndef MAUENG_SERVICELOCATOR_H
#define MAUENG_SERVICELOCATOR_H

#include "Renderer.h"
#include "GameTime.h"

#include <memory>

namespace MauEng
{
	class ServiceLocator final
	{
	public:
		[[nodiscard]] static MauRen::Renderer& GetRenderer() { return (*m_pRenderer); }
		static void RegisterRenderer(std::unique_ptr<MauRen::Renderer>&& pRenderer)
		{
			m_pRenderer = ((!pRenderer) ? std::make_unique<MauRen::NullRenderer>(nullptr) : std::move(pRenderer));
		}

		[[nodiscard]] static MauEng::Time& GetTime() { return MauEng::Time::GetInstance(); }


	private:
		static std::unique_ptr<MauRen::Renderer> m_pRenderer;
	};

	// Helper function for easy access to the renderer
	inline MauRen::Renderer& Renderer()
	{
		return ServiceLocator::GetRenderer();
	}

	// Helper function for easy access to the time
	inline MauEng::Time& Time()
	{
		return ServiceLocator::GetTime();
	}
}

#endif