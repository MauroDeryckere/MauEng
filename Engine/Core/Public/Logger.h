#ifndef MAUCOR_LOGGER_H
#define MAUCOR_LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <concepts>
#include <mutex>

#include <format>
#include <fmt/format.h>

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

	//TODO log categories

	class Logger
	{
	public:
		virtual ~Logger() = default;

		Logger(Logger const&) = delete;
		Logger(Logger&&) = delete;
		Logger& operator=(Logger const&) = delete;
		Logger& operator=(Logger&&) = delete;

		template<typename... Args>
		void Log(LogPriority priority, fmt::format_string<Args...> fmtStr, Args... args)
		{
			if (priority >= m_LogPriority)
			{
				return;
			}

			std::scoped_lock lock{ m_Mutex };
			LogInternal(priority, fmt::format(fmtStr, std::forward<Args>(args)...));
		}

		inline void SetPriorityLevel(LogPriority priority) noexcept;

	protected:
		Logger() = default;

		virtual void LogInternal(LogPriority priority, std::string const& message) = 0;

	private:
		LogPriority m_LogPriority{ LogPriority::Fatal };
		mutable std::mutex m_Mutex{};

		// The std way of doing the logging, we're using fmt now but keeping this fnction in case we want to go back
		template<typename... Args>
		std::string Format(char const* fmt, Args&&... args)
		{
			return std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
		}
	};


	class NullLogger final : public Logger
	{
	public:
		NullLogger() = default;
		virtual ~NullLogger() override = default;

		NullLogger(NullLogger const&) = delete;
		NullLogger(NullLogger&&) = delete;
		NullLogger& operator=(NullLogger const&) = delete;
		NullLogger& operator=(NullLogger&&) = delete;

	protected:

	private:
		virtual void LogInternal(LogPriority priority, std::string const& message) override {}
	};
}

#endif