#include "Profiling/OptickProfiler.h"

// must be included before optick.h to control macros
#include "Config/EngineConfig.h"
#include <Optick.h>

namespace MauCor
{
	void OptickProfiler::BeginSession(std::string const& name, std::string& filepath, size_t reserveSize)
	{
		filepath += ".opt";
		m_Path = filepath;
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
		Profiler::FixFilePath(m_Path.c_str());

		OPTICK_STOP_CAPTURE()
		OPTICK_SAVE_CAPTURE(m_Path.c_str())
	}
}
