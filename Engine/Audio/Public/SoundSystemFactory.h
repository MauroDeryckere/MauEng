#ifndef MAUDIO_SOUNDSYSTEMFACTORY_H
#define MAUDIO_SOUNDSYSTEMFACTORY_H

#include "SoundSystem.h"
#include <memory>

namespace MAudio
{
	[[nodiscard]] std::unique_ptr<SoundSystem> CreateNullSoundSystem() noexcept;
	[[nodiscard]] std::unique_ptr<SoundSystem> CreateWWiseSoundSystem() noexcept;
}

#endif