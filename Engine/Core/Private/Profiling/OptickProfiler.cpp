#include "Profiling/OptickProfiler.h"

// must be included before optick.h to control macros
#include "Config/EngineConfig.h"
#include <Optick.h>

namespace MauCor
{
	void OptickProfiler::BeginSessionInternal(std::string const& name, size_t reserveSize)
	{
		fileName += ".opt";

		OPTICK_START_CAPTURE()
	}

	void OptickProfiler::WriteProfile(ProfileResult const& result, bool isFunction)
	{
		ME_CHECK(false);
	}

	void OptickProfiler::WriteProfile(std::string const& name)
	{
		ME_CHECK(false);
	}

	void OptickProfiler::EndSession()
	{
		Profiler::FixFilePath(fileName.c_str());

		OPTICK_STOP_CAPTURE()
		OPTICK_SAVE_CAPTURE(fileName.c_str())
	}
}
