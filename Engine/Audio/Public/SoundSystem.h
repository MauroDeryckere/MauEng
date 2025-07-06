#ifndef MAUDIO_SOUNDSYSTEM_H
#define MAUDIO_SOUNDSYSTEM_H

#ifdef _NDEBUG
	#define AK_OPTIMIZED
#endif

#ifdef WIN32
	#define AK_WIN
#endif

namespace MAudio
{
	class SoundSystem
	{
	public:
		SoundSystem() = default;
		virtual ~SoundSystem() = default;

		virtual bool Initialize() noexcept = 0;
		virtual bool Destroy() noexcept = 0; 
	};
}

#endif