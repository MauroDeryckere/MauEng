#ifndef MAUCOR_CONSOLELOGGER_H
#define MAUCOR_CONSOLELOGGER_H

#include "Logger.h"

namespace MauCor
{
	class ConsoleLogger final : public Logger
	{
	public:
		ConsoleLogger() = default;
		virtual ~ConsoleLogger() override = default;

		ConsoleLogger(ConsoleLogger const&) = delete;
		ConsoleLogger(ConsoleLogger&&) = delete;
		ConsoleLogger& operator=(ConsoleLogger const&) = delete;
		ConsoleLogger& operator=(ConsoleLogger&&) = delete;
	private:
		virtual void LogInternal(LogPriority priority, LogCategory category, std::string const& message) override;
	};
}

#endif