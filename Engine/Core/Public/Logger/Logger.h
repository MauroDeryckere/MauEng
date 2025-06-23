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
#include "LogCategories.h"

namespace MauCor
{
	class Logger
	{
	public:
		virtual ~Logger() = default;

		Logger(Logger const&) = delete;
		Logger(Logger&&) = delete;
		Logger& operator=(Logger const&) = delete;
		Logger& operator=(Logger&&) = delete;

		template<typename... Args>
		void Log(ELogPriority priority, ELogCategory category, fmt::format_string<Args...> fmtStr, Args... args)
		{
			if (priority < m_LogPriority)
			{
				return;
			}

			std::scoped_lock lock{ m_Mutex };
			LogInternalEnumBased(priority, category, fmt::format(fmtStr, std::forward<Args>(args)...));
		}
		template<typename... Args>
		void Log(ELogPriority priority, std::string_view const category, fmt::format_string<Args...> fmtStr, Args... args)
		{
			if (priority < m_LogPriority)
			{
				return;
			}

			std::scoped_lock lock{ m_Mutex };
			LogInternal(priority, category, fmt::format(fmtStr, std::forward<Args>(args)...));
		}
		template<typename... Args>
		void Log(ELogPriority priority, LogCategory const& category, fmt::format_string<Args...> fmtStr, Args... args)
		{
			if (priority < m_LogPriority)
			{
				return;
			}

			std::scoped_lock lock{ m_Mutex };
			LogInternal(priority, category.GetName(), fmt::format(fmtStr, std::forward<Args>(args)...));
		}

		void SetPriorityLevel(ELogPriority priority) noexcept;

	protected:
		Logger() = default;

		virtual void LogInternal(ELogPriority priority, std::string_view category, std::string_view const message) = 0;
		static constexpr char const* PriorityToString(ELogPriority priority) noexcept
		{
			switch (priority)
			{
				case ELogPriority::Trace: return "Trace";
				case ELogPriority::Info: return "Info";
				case ELogPriority::Debug: return "Debug";
				case ELogPriority::Warn: return "Warn";
				case ELogPriority::Error: return "Error";
				case ELogPriority::Fatal: return "Fatal";

				default: return "Unknown";
			}
		}

		static constexpr char const* PriorityToColour(ELogPriority priority) noexcept
		{
			switch (priority)
			{
			case ELogPriority::Trace: return MauEng::LOG_COLOR_TRACE;
			case ELogPriority::Info: return MauEng::LOG_COLOR_INFO;
			case ELogPriority::Debug: return MauEng::LOG_COLOR_DEBUG;
			case ELogPriority::Warn: return MauEng::LOG_COLOR_WARNING;
			case ELogPriority::Error: return MauEng::LOG_COLOR_ERROR;
			case ELogPriority::Fatal: return MauEng::LOG_COLOR_FATAL;

			default: return MauEng::LOG_COLOR_RESET;
			}
		}

		static constexpr char const* CategoryToString(ELogCategory category) noexcept
		{
			switch (category)
			{
				case ELogCategory::Core: return "Core";
				case ELogCategory::Engine: return "Engine";
				case ELogCategory::Renderer: return "Renderer";
				case ELogCategory::Game: return "Game";

				default: return "Unknown";
			}
		}
	private:
		ELogPriority m_LogPriority{ LOG_STRIP_LEVEL };
		mutable std::mutex m_Mutex{};

		// The std way of doing the logging, we're using fmt now but keeping this fnction in case we want to go back
		template<typename... Args>
		std::string Format(char const* fmt, Args&&... args)
		{
			return std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
		}

		void LogInternalEnumBased(ELogPriority priority, ELogCategory category, std::string const& message)
		{
			LogInternal(priority, CategoryToString(category), message);
		}
	};
}

#endif