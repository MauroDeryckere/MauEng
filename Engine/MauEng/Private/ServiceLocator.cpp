#include "ServiceLocator.h"
#include "RendererFactory.h"
#include "SoundSystemFactory.h"

namespace MauEng
{
	std::unique_ptr<MauRen::DebugRenderer> ServiceLocator::m_pDebugRenderer{ MauRen::CreateDebugRenderer(true) };
	std::unique_ptr<MAudio::SoundSystem> ServiceLocator::m_pSoundSystem{ MAudio::CreateNullSoundSystem() };

	void ServiceLocator::RegisterDebugRenderer(std::unique_ptr<MauRen::DebugRenderer>&& pRenderer)
	{
		m_pDebugRenderer = ((!pRenderer) ? MauRen::CreateDebugRenderer(true) : std::move(pRenderer));
	}

	void ServiceLocator::RegisterSoundSystem(std::unique_ptr<MAudio::SoundSystem>&& pSoundSystem)
	{
		if (pSoundSystem)
		{
			m_pSoundSystem = std::move(pSoundSystem);
		}
		else
		{
			m_pSoundSystem = MAudio::CreateNullSoundSystem();
		}
	}
}
