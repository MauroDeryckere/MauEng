#ifndef MAUCOR_LOGCATEGORIES_H
#define MAUCOR_LOGCATEGORIES_H

namespace MauCor
{
	enum class LogPriority : uint8_t
	{
		Trace,
		Info,
		Debug,
		Warn,
		Error,
		Fatal
	};

	enum class LogCategory : uint8_t
	{
		Core,
		Engine,
		Renderer,
		Game
	};
}

MauCor::LogPriority constexpr Trace{ MauCor::LogPriority::Trace };
MauCor::LogPriority constexpr Info{ MauCor::LogPriority::Info };
MauCor::LogPriority constexpr Debug{ MauCor::LogPriority::Debug };
MauCor::LogPriority constexpr Warn{ MauCor::LogPriority::Warn };
MauCor::LogPriority constexpr Error{ MauCor::LogPriority::Error };
MauCor::LogPriority constexpr Fatal{ MauCor::LogPriority::Fatal };

#endif