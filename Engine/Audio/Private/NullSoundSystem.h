#ifndef MAUDIO_NULLSOUNDSYSTEM_H
#define MAUDIO_NULLSOUNDSYSTEM_H

#include "SoundSystem.h"

namespace MAudio
{
	class NullSoundSystem final : public SoundSystem
	{
	public:
		NullSoundSystem() = default;
		virtual ~NullSoundSystem() override = default;
	};
}

#endif