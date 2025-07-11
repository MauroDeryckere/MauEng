#ifndef MAUCOR_CONSOLELOGGER_H
#define MAUCOR_CONSOLELOGGER_H

#include "Logger/Logger.h"

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
		virtual void LogInternal(ELogPriority priority, std::string_view const category, std::string_view const message) override;
	};
}

#endif