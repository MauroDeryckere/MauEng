#ifndef MAUCOR_CORESERVICELOCATOR_H
#define MAUCOR_CORESERVICELOCATOR_H

#include <memory>

#include "Logger/Logger.h"
#include "GameTime.h"

#include <Profiler/ProfilerMacros.h>

namespace MauCor
{
	class CoreServiceLocator final
	{
	public:
		[[nodiscard]] static Logger& GetLogger() { return (*m_pLogger); }
		static void RegisterLogger(std::unique_ptr<Logger>&& pLogger);

		[[nodiscard]] static MauCor::Time& GetTime() { return MauCor::Time::GetInstance(); }

	private:
		static std::unique_ptr<Logger> m_pLogger;
	};

#define TIME MauCor::CoreServiceLocator::GetTime()

#pragma region EasyAccessHelpers
#define LOGGER MauCor::CoreServiceLocator::GetLogger()
	#define ME_LOG(priority, category, fmtStr, ...) \
			do { \
				if constexpr (priority >= LOG_STRIP_LEVEL) { \
					if (category.GetPriority() >= LOG_STRIP_LEVEL) { \
						LOGGER.Log(priority, category, fmtStr, ##__VA_ARGS__); \
					} \
				}  \
			} while (false)

	#define ME_LOG_TRACE(category, fmtStr, ...) ME_LOG(MauCor::ELogPriority::Trace, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_INFO(category, fmtStr, ...) ME_LOG(MauCor::ELogPriority::Info, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_DEBUG(category, fmtStr, ...) ME_LOG(MauCor::ELogPriority::Debug, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_WARN(category, fmtStr, ...) ME_LOG(MauCor::ELogPriority::Warn, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_ERROR(category, fmtStr, ...) ME_LOG(MauCor::ELogPriority::Error, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_FATAL(category, fmtStr, ...) ME_LOG(MauCor::ELogPriority::Fatal, category, fmtStr, __VA_ARGS__)

	#define ME_PROFILE_BEGIN_SESSION(name, filepath, ...) PROFILER_BEGIN_SESSION(name, filepath, ##__VA_ARGS__);
	#define ME_PROFILE_END_SESSION() PROFILER_END_SESSION();
	#define ME_PROFILE_FUNCTION() PROFILER_FUNCTION();
	#define ME_PROFILE_SCOPE(name) PROFILER_SCOPE(name);
	#define ME_PROFILE_THREAD(name) PROFILER_THREAD(name);
	#define ME_PROFILE_FRAME() PROFILER_FRAME("MainThread");
	#define ME_PROFILE_TICK() PROFILER_TICK();
#pragma endregion
}

#endif