#include "CoreServiceLocator.h"

#include "Logger/NullLogger.h"

#include "Profiling/NullProfiler.h"
#include "Profiling/OptickProfiler.h"
#include "Profiling/GoogleProfiler.h"

namespace MauCor
{
	std::unique_ptr<Logger> CoreServiceLocator::m_pLogger{ std::make_unique<NullLogger>() };

#if ENABLE_PROFILER
	#if USE_OPTICK
		std::unique_ptr<Profiler> CoreServiceLocator::m_pProfiler{ std::make_unique<OptickProfiler>() };
	#else
		std::unique_ptr<Profiler> CoreServiceLocator::m_pProfiler{ std::make_unique<GoogleProfiler>() };
	#endif
#else
	std::unique_ptr<Profiler> CoreServiceLocator::m_pProfiler{ std::make_unique<NullProfiler>() };
#endif

	void CoreServiceLocator::RegisterLogger(std::unique_ptr<Logger>&& pLogger)
	{
		m_pLogger = ((!pLogger) ? std::make_unique<NullLogger>() : std::move(pLogger));
	}

	void CoreServiceLocator::RegisterProfiler(std::unique_ptr<Profiler>&& pProfiler)
	{
		m_pProfiler = ((!pProfiler) ? std::make_unique<NullProfiler>() : std::move(pProfiler));
	}
}
