#ifndef MAUCOR_CORESERVICELOCATOR_H
#define MAUCOR_CORESERVICELOCATOR_H

#include <memory>

#include "Logger/Logger.h"
#include "Profiling/Profiler.h"
#include "Profiling/InstrumentorTimer.h"

#if USE_OPTICK
	#include <optick.h>
#endif

namespace MauCor
{
	class CoreServiceLocator final
	{
	public:
		[[nodiscard]] static Logger& GetLogger() { return (*m_pLogger); }
		static void RegisterLogger(std::unique_ptr<Logger>&& pLogger);

		[[nodiscard]] static Profiler& GetProfiler() { return (*m_pProfiler); }
		static void RegisterProfiler(std::unique_ptr<Profiler>&& pProfiler);

	private:
		static std::unique_ptr<Logger> m_pLogger;
		static std::unique_ptr<Profiler> m_pProfiler;
	};

#pragma region EasyAccessHelpers
#define LOGGER MauCor::CoreServiceLocator::GetLogger()
	#define ME_LOG(priority, category, fmtStr, ...) \
			do { \
				if constexpr (priority >= LOG_STRIP_LEVEL) { \
					LOGGER.Log(priority, category, fmtStr, ##__VA_ARGS__); \
				} \
			} while (false)

	#define ME_LOG_TRACE(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Trace, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_INFO(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Info, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_DEBUG(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Debug, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_WARN(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Warn, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_ERROR(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Error, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_FATAL(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Fatal, category, fmtStr, __VA_ARGS__)

#define PROFILER MauCor::CoreServiceLocator::GetProfiler()
	#define CONCAT(x, y) x ## y
	#define C(x, y) CONCAT(x, y)

	#define ME_PROFILE_BEGIN_SESSION(name, filepath, ...) PROFILER.BeginSession(name, filepath, __VA_ARGS__);
	#define ME_PROFILE_END_SESSION() PROFILER.EndSession();
#if USE_OPTICK
	#define ME_PROFILE_FUNCTION() OPTICK_EVENT();
	#define ME_PROFILE_SCOPE(name) OPTICK_EVENT(name);

	#define ME_PROFILE_THREAD(name) OPTICK_THREAD(name)
	#define ME_PROFILE_FRAME() OPTICK_FRAME("MainThread")
#else
	#define ME_PROFILE_SCOPE(name) MauCor::InstrumentorTimer C(timer, __LINE__) { name, false };
	#define ME_PROFILE_FUNCTION() MauCor::InstrumentorTimer C(timer, __LINE__) { __FUNCTION__, true };

	#define ME_PROFILE_THREAD(name)
	#define ME_PROFILE_FRAME()
#endif
#pragma endregion
}

#endif