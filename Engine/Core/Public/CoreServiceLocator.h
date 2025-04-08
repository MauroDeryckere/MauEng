#ifndef MAUCOR_CORESERVICELOCATOR_H
#define MAUCOR_CORESERVICELOCATOR_H

#include <memory>
#include "Logger.h"

namespace MauCor
{
	class CoreServiceLocator final
	{
	public:
		[[nodiscard]] static Logger& GetLogger() { return (*m_pLogger); }
		static void RegisterLogger(std::unique_ptr<Logger>&& pLogger)
		{
			m_pLogger = ((!pLogger) ? std::make_unique<NullLogger>() : std::move(pLogger));
		}

	private:
		static std::unique_ptr<Logger> m_pLogger;
	};

#pragma region EasyAccessHelpers
	#define LOGGER MauCor::CoreServiceLocator::GetLogger()

	#define ME_LOG(priority, category, fmtStr, ...) \
				LOGGER.Log(priority, category, fmtStr, __VA_ARGS__)

	#define ME_LOG_TRACE(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Trace, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_INFO(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Info, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_DEBUG(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Debug, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_WARN(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Warn, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_ERROR(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Error, category, fmtStr, __VA_ARGS__)
	#define ME_LOG_FATAL(category, fmtStr, ...) ME_LOG(MauCor::LogPriority::Fatal, category, fmtStr, __VA_ARGS__)

#pragma endregion
}

#endif