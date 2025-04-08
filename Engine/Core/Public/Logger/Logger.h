#ifndef MAUCOR_LOGGER_H
#define MAUCOR_LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <concepts>
#include <mutex>

#include <format>
#include <fmt/format.h>

#include "Config/EngineConfig.h"

#include "LoggerFactory.h"

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

	class Logger
	{
	public:
		virtual ~Logger() = default;

		Logger(Logger const&) = delete;
		Logger(Logger&&) = delete;
		Logger& operator=(Logger const&) = delete;
		Logger& operator=(Logger&&) = delete;

		template<typename... Args>
		void Log(LogPriority priority, LogCategory category, fmt::format_string<Args...> fmtStr, Args... args)
		{
			if (priority < m_LogPriority)
			{
				return;
			}

			std::scoped_lock lock{ m_Mutex };
			LogInternal(priority, category, fmt::format(fmtStr, std::forward<Args>(args)...));
		}

		void SetPriorityLevel(LogPriority priority) noexcept;

	protected:
		Logger() = default;

		virtual void LogInternal(LogPriority priority, LogCategory category, std::string const& message) = 0;
		static constexpr char const* PriorityToString(LogPriority priority) noexcept
		{
			switch (priority)
			{
				case LogPriority::Trace: return "Trace";
				case LogPriority::Info: return "Info";
				case LogPriority::Debug: return "Debug";
				case LogPriority::Warn: return "Warn";
				case LogPriority::Error: return "Error";
				case LogPriority::Fatal: return "Fatal";

				default: return "Unknown";
			}
		}

		static constexpr char const* PriorityToColour(LogPriority priority) noexcept
		{
			switch (priority)
			{
			case LogPriority::Trace: return MauEng::LOG_COLOR_TRACE;
			case LogPriority::Info: return MauEng::LOG_COLOR_INFO;
			case LogPriority::Debug: return MauEng::LOG_COLOR_DEBUG;
			case LogPriority::Warn: return MauEng::LOG_COLOR_WARNING;
			case LogPriority::Error: return MauEng::LOG_COLOR_ERROR;
			case LogPriority::Fatal: return MauEng::LOG_COLOR_FATAL;

			default: return MauEng::LOG_COLOR_RESET;
			}
		}

		static constexpr char const* CategoryToString(LogCategory category) noexcept
		{
			switch (category)
			{
				case LogCategory::Core: return "Core";
				case LogCategory::Engine: return "Engine";
				case LogCategory::Renderer: return "Renderer";
				case LogCategory::Game: return "Game";

				default: return "Unknown";
			}
		}
	private:
		LogPriority m_LogPriority{ LogPriority::Trace };
		mutable std::mutex m_Mutex{};

		// The std way of doing the logging, we're using fmt now but keeping this fnction in case we want to go back
		template<typename... Args>
		std::string Format(char const* fmt, Args&&... args)
		{
			return std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
		}
	};
}

#endif