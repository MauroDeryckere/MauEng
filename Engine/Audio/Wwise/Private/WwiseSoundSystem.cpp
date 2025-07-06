#include "WwiseSoundSystem.h"

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>  

// Memory
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>

// Streaming
#include <AK/SoundEngine/Common/IAkStreamMgr.h> 
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>

#include <AK/SpatialAudio/Common/AkSpatialAudio.h>

#include "../../../Core/Shared/AssertsInternal.h"

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>
#endif

namespace MAudio
{
	bool WwiseSoundSystem::Initialize() noexcept
	{
		AkMemSettings memSettings;
		AK::MemoryMgr::GetDefaultSettings(memSettings);

		// Customize the mem settings here.

		if (AK_Success != AK::MemoryMgr::Init(&memSettings))
		{
			ME_ENGINE_ASSERT(false, "Could not create the memory manager.");
			return false;
		}

		// Create and initialize an instance of the default streaming manager. 
		// Note that you can override the default streaming manager with your own. 
		AkStreamMgrSettings stmSettings;
		AK::StreamMgr::GetDefaultSettings(stmSettings);

		// Customize the Stream Manager settings here.

		if (not AK::StreamMgr::Create(stmSettings))
		{
			ME_ENGINE_ASSERT(false, "Could not create the Streaming Manager");
			return false;
		}

		//TODO
		// Create a streaming device.
		// Note that you can override the default low-level I/O module with your own. 
		AkDeviceSettings deviceSettings;
		AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

		// Customize the streaming device settings here.

		//CAkFilePackageLowLevelIODeferred g_lowLevelIO;
		//// CAkFilePackageLowLevelIODeferred::Init() creates a streaming device
		//// in the Stream Manager, and registers itself as the File Location Resolver.
		//if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
		//{
		//	ME_ENGINE_ASSERT(false, "Could not create the streaming device and Low-Level I/O system");
		//	return false;
		//}


		// Create the Sound Engine
		AkInitSettings initSettings;
		AK::SoundEngine::GetDefaultInitSettings(initSettings);
		AkPlatformInitSettings platformInitSettings;
		AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

		if (AK_Success != AK::SoundEngine::Init(&initSettings, &platformInitSettings))
		{
			ME_ENGINE_ASSERT(false, "Could not initialize the Sound Engine.");
			return false;
		}

		AkSpatialAudioInitSettings settings{};
		if (AK_Success != AK::SpatialAudio::Init(settings))
		{
			ME_ENGINE_ASSERT(false, "Could not initialize the Spatial Audio.");
			return false;
		}

		#ifndef AK_OPTIMIZED
			AkCommSettings commSettings;
			AK::Comm::GetDefaultInitSettings(commSettings);
			if (AK::Comm::Init(commSettings) != AK_Success)
			{
				ME_ENGINE_ASSERT(false, "Could not initialize communication.");
				return false;
			}
		#endif

		return true;
	}

	bool WwiseSoundSystem::Destroy() noexcept
	{
		#ifndef AK_OPTIMIZED
			AK::Comm::Term();
		#endif

		//AK::SpatialAudio::Term();

		AK::MusicEngine::Term();
		AK::SoundEngine::Term();

		//g_lowLevelIO.Term();

		if (AK::IAkStreamMgr::Get())
		{
			AK::IAkStreamMgr::Get()->Destroy();
		}

		AK::MemoryMgr::Term();

		return true;
	}
}
