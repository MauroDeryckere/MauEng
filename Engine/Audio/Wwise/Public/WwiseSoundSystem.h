#ifndef MAUDIO_WWISESOUNDSYSTEM_H
#define MAUDIO_WWISESOUNDSYSTEM_H

#include "SoundSystem.h"

namespace MAudio
{
	class WwiseSoundSystem final : public SoundSystem
	{
	public:
		WwiseSoundSystem()  = default;
		virtual ~WwiseSoundSystem() override = default;

		virtual bool Initialize() noexcept override;
		virtual bool Destroy() noexcept override;
	};
}

#endif