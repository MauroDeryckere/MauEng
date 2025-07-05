#include "SoundSystemFactory.h"

#include "Defines.h"

#include "NullSoundSystem.h"

#if BUILD_WWISE
	#include "WwiseSoundSystem.h"
#endif

namespace MAudio
{
	std::unique_ptr<SoundSystem> CreateNullSoundSystem() noexcept
	{
		return std::make_unique<NullSoundSystem>();
	}

	std::unique_ptr<SoundSystem> CreateWWiseSoundSystem() noexcept
	{
	#if BUILD_WWISE
		return std::make_unique<WwiseSoundSystem>();
	#else
		return CreateNullSoundSystem();
	#endif
	}
}


